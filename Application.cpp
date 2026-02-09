#include "Application.h"

#include <Antilatency.InterfaceContract.LibraryLoader.h>
#include <Antilatency.InterfaceContract.PathResolver.h>

#include <iostream>
#include <chrono>

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

int Application::run() {
    auto targetNode = waitTargetDevice();

    if (targetNode == Antilatency::DeviceNetwork::NodeHandle::Null) {
        std::cout << "Failed to find target device" << std::endl;
        return -1;
    }

    if (targetNodeContainRequiredProperties(targetNode)) {

    }

    return 0;
}

Antilatency::DeviceNetwork::NodeHandle Application::waitTargetDevice() {
    using namespace Antilatency::DeviceNetwork;
    NodeHandle result {NodeHandle::Null};
    auto start = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();

    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    while (duration_ms < settings.waitDeviceTimeout_ms) {
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

    for (const auto& [key, value] : configSettings.targetDeviceProperties) {
        if (!result) {
            break;
        }
        try {
            result &= _deviceNetwork.nodeGetStringProperty(node, key) == value;
        } catch (...) {
            result &= false;
        }
    }

    return result;
}

bool Application::targetNodeContainRequiredProperties(const Antilatency::DeviceNetwork::NodeHandle& node) {
    bool result {false};

    return result;
}