#include "Application.h"
#include "ThreadSafeQueue.h"

#include <Antilatency.InterfaceContract.LibraryLoader.h>
#include <Antilatency.InterfaceContract.PathResolver.h>

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <thread>

#include <csv2/writer.hpp>

Application::Application(const ConfigSettings& configSettings, const ApplicationSettings& settings) : settings(std::move(settings)), configSettings(std::move(configSettings)) {
    using namespace Antilatency;
    auto deviceNetworkLibrary = InterfaceContract::getLibraryInterface<DeviceNetwork::ILibrary>(InterfaceContract::PathResolver::siblingLibraryPath("AntilatencyDeviceNetwork").c_str());

    if (!deviceNetworkLibrary) {
        throw std::runtime_error("Failed to load AntilatencyDeviceNetwork library");
    }

    std::cout << "Load AntilatencyDeviceNetwork library " << deviceNetworkLibrary.getVersion() << std::endl;

    auto filter = deviceNetworkLibrary.createFilter();
    filter.addUsbDevice(DeviceNetwork::Constants::AllUsbDevices);
    if (settings.enableIpDevices) {
        std::cout << "Add All IP devices to device network filter" << std::endl;
        filter.addIpDevice(DeviceNetwork::Constants::AllIpDevicesIp, DeviceNetwork::Constants::AllIpDevicesMask);
    }

    _deviceNetwork = deviceNetworkLibrary.createNetwork(filter);

    if (!_deviceNetwork) {
        throw std::runtime_error("Failed to create device network");
    }
}

std::string getCurrentDateAndTime(const char* timeFormat = "%d_%m_%Y_%H_%M_%S") {
    auto n = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(n);
    std::tm buf;
    localtime_s(&buf, &in_time_t);
    std::stringstream oss{};
    oss << std::put_time(&buf, timeFormat);
    return oss.str();
}

int Application::run() {
    auto targetNode = waitTargetDevice();

    if (targetNode == Antilatency::DeviceNetwork::NodeHandle::Null) {
        std::cout << "Failed to find target device" << std::endl;
        return -1;
    }

    if (!targetNodeContainRequiredProperties(targetNode)) {
        return -1;
    }

    std::string fileName{getCurrentDateAndTime() + ".csv"};
    std::ofstream csvFileStream{fileName};

    if (!csvFileStream.is_open()) {
        std::cout << "Failed to create file " << fileName << std::endl;
    }

    csv2::Writer<csv2::delimiter<';'>> csvWriter{csvFileStream};

    {
        std::vector<std::string> csvHead{"Timestamp", "ElapsedTime_ms"};
        for (const auto& property : configSettings.dumpProperties) {
            if (property.alias.empty()) {
                csvHead.emplace_back(property.propertyName);
            } else {
                csvHead.emplace_back(property.alias);
            }
        }
        csvWriter.write_row(csvHead);
    }

    TSQueue<std::vector<std::string>> queue{};
    std::thread writeToFile{[&] {
        try {
            for (std::size_t i = 0; i < settings.samples_count; ++i) {
                auto properties = queue.pop();
                csvWriter.write_row(properties);

                if (settings.printProgress) {
                    std::cout << "Iteration " << (i + 1) << "/" << settings.samples_count << " completed" << std::endl;
                }
            }
        } catch (const std::exception& exp) {
            std::cout << "Failed to write file with error: " << exp.what() << std::endl;
        }
    }};

    std::vector<std::string> properties{};
    auto startDumpTime = std::chrono::steady_clock::now();
    try {
        for (std::size_t i = 0; i < settings.samples_count; ++i) {
            auto beforeDump = std::chrono::steady_clock::now();
            auto propertyManager = _deviceNetwork.nodeStartPropertyTask(targetNode);

            properties.emplace_back(getCurrentDateAndTime("%d/%m/%Y%H:%M:%S"));
            properties.emplace_back(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(beforeDump - startDumpTime).count()));

            if (!propertyManager) {
                std::cout << "Failed to create property manager" << std::endl;
                break;
            }

            for (const auto& property : configSettings.dumpProperties) {
                properties.push_back(propertyManager.getStringProperty(property.propertyName));
            }

            queue.push(properties);
            properties.clear();

            auto sleep_until = beforeDump + std::chrono::milliseconds(settings.period_ms);

            std::this_thread::sleep_until(sleep_until);
        }
    } catch (const std::exception& exp) {
        std::cout << "Failed to dump with error: " << exp.what() << std::endl;
    }

    if (writeToFile.joinable()) {
        writeToFile.join();
    }

    csvFileStream.close();

    return 0;
}

Antilatency::DeviceNetwork::NodeHandle Application::waitTargetDevice() {
    using namespace Antilatency::DeviceNetwork;
    NodeHandle result{NodeHandle::Null};
    auto start = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();

    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    while (static_cast<std::size_t>(duration_ms) < settings.waitDeviceTimeout_ms) {
        now = std::chrono::steady_clock::now();
        duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        const auto nodes = _deviceNetwork.getNodes();
        for (const auto& node : nodes) {
            if (_deviceNetwork.nodeGetStatus(node) != NodeStatus::Idle) {
                continue;
            }
            if (isTargetNode(node)) {
                result = node;
                break;
            }
        }
        if (result != NodeHandle::Null) {
            break;
        }
    }

    return result;
}

bool Application::isTargetNode(const Antilatency::DeviceNetwork::NodeHandle& node) {
    using namespace Antilatency::DeviceNetwork;

    bool result{node != NodeHandle::Null};

    try {
        auto propertyTask = _deviceNetwork.nodeStartPropertyTask(node);
        result &= propertyTask;
        for (const auto& [key, value] : configSettings.targetDeviceProperties) {
            if (!result) {
                break;
            }
            result &= _deviceNetwork.nodeGetStringProperty(node, key) == value;
        }
    } catch (const std::exception& exp) {
        std::cout << "Failed to get property with error " << exp.what() << std::endl;
        result &= false;
    }

    return result;
}

bool Application::targetNodeContainRequiredProperties(const Antilatency::DeviceNetwork::NodeHandle& node) {
    using namespace Antilatency::DeviceNetwork;
    if (node == NodeHandle::Null) {
        return false;
    }

    try {
        auto propertyTask = _deviceNetwork.nodeStartPropertyTask(node);
        if (!propertyTask) {
            return false;
        }
        for (const auto& property : configSettings.dumpProperties) {
            propertyTask.getStringProperty(property.propertyName);
        }
    } catch (const std::exception& exp) {
        std::cout << "Failed to get property with error " << exp.what() << std::endl;
        return false;
    }

    return true;
}