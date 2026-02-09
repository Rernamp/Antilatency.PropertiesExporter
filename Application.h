#pragma once

#include "Application.Settings.h"

#include <Antilatency.Api.h>

struct Application {
    Application(const ConfigSettings& configSettings, const ApplicationSettings& settings);

    int run();
    const ApplicationSettings settings;
    const ConfigSettings configSettings;
private:
    Antilatency::DeviceNetwork::NodeHandle waitTargetDevice();
    bool isTargetNode(const Antilatency::DeviceNetwork::NodeHandle& node);
    bool targetNodeContainRequiredProperties(const Antilatency::DeviceNetwork::NodeHandle& node);
private:
    Antilatency::DeviceNetwork::INetwork _deviceNetwork {nullptr};
};