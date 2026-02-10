#include "Application.h"

#include <Antilatency.InterfaceContract.LibraryLoader.h>
#include <Antilatency.InterfaceContract.PathResolver.h>

#include <iostream>
#include <fstream>
#include <chrono>
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

std::string getCurrentDateAndTime() {
    const auto t = std::time(nullptr);
    const auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
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

    std::string fileName {getCurrentDateAndTime() + ".csv"};
    std::ofstream csvFileStream {fileName};

    if (!csvFileStream.is_open()) {
        std::cout << "Failed to create file " << fileName << std::endl;
    }

    csv2::Writer<csv2::delimiter<';'>> csvWriter {csvFileStream};

    std::vector<std::string> properties {configSettings.dumpProperties.size()};
    try {
        for (std::size_t i = 0; i < settings.samples_count; ++i) {
            auto propertyManager = _deviceNetwork.nodeStartPropertyTask(targetNode);

            if (!propertyManager) {
                std::cout << "Failed to create property manager" << std::endl;
                break;
            }

            for (const auto& key : configSettings.dumpProperties) {
                properties.push_back(propertyManager.getStringProperty(key));
            }

            csvWriter.write_row(properties);
            properties.clear();

            std::this_thread::sleep_for(std::chrono::milliseconds(settings.period_ms));
        }
    } catch (const std::exception& exp) {
        std::cout << "Failed to dump with error: " << exp.what() << std::endl;
    }

    csvFileStream.close();

    return 0;
}

Antilatency::DeviceNetwork::NodeHandle Application::waitTargetDevice() {
    using namespace Antilatency::DeviceNetwork;
    NodeHandle result {NodeHandle::Null};
    auto start = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();

    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    while (static_cast<std::size_t>(duration_ms) < settings.waitDeviceTimeout_ms) {
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
    }

    return result;
}

bool Application::isTargetNode(const Antilatency::DeviceNetwork::NodeHandle& node) {
    using namespace Antilatency::DeviceNetwork;

    bool result{node == NodeHandle::Null};

    try {
        auto propertyTask = _deviceNetwork.nodeStartPropertyTask(node);
        result &= !propertyTask;
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
            propertyTask.getStringProperty(property);
        }
    } catch (const std::exception& exp) {
        std::cout << "Failed to get property with error " << exp.what() << std::endl;
        return false;
    }

    return true;
}