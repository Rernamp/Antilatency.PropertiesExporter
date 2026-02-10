#include "Application.Settings.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void from_json(const json& j, ConfigSettings& settings) {
    j.at("targetDeviceProperties").get_to(settings.targetDeviceProperties);
    j.at("dumpProperties").get_to(settings.dumpProperties);
}

ConfigSettings parseFromString(const std::string& str) {
    auto json = json::parse(str);
    return json.get<ConfigSettings>();
}