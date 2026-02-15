#pragma once

#include <string>
#include <map>
#include <vector>

struct Property {
    std::string propertyName {};
    std::string alias {};
};
struct ConfigSettings {
    std::map<std::string, std::string> targetDeviceProperties {};
    std::vector<Property> dumpProperties {};
};

struct ApplicationSettings {
    std::size_t period_ms {1};
    std::size_t samples_count {10};
    std::size_t waitDeviceTimeout_ms {2000};
    bool printProgress {false};
    bool enableIpDevices {false};
};

ConfigSettings parseFromString(const std::string& str);

