#include "Application.Settings.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void from_json(const json& j, ConfigSettings& settings) {
    j.at("targetDeviceProperties").get_to(settings.targetDeviceProperties);
}

ConfigSettings parseFromString(const std::string& str) {
    auto json = json::parse(str);
    return json.get<ConfigSettings>();
}