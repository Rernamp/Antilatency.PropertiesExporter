cmake_minimum_required(VERSION 3.10)

project(Antilatency.DeviceNetwork.PropertiesExporter)

set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_SOURCE_DIR}/AntilatencySdk/Api)
add_executable(Antilatency.DeviceNetwork.PropertiesExporter)

add_library(Application INTERFACE)

target_sources(Application INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.Settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.Settings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ThreadSafeQueue.h
)

target_link_libraries(Antilatency.DeviceNetwork.PropertiesExporter PRIVATE
    Application
    CLI11::CLI11
    Antilatency::SDK
    nlohmann_json::nlohmann_json
    csv2::csv2
)


add_subdirectory(Dependencies)
include(Dependencies/AntilatencySDK.Cmake/CopySDK.cmake)

copy_antilatency_sdk(Antilatency.DeviceNetwork.PropertiesExporter)

add_custom_command(TARGET Antilatency.DeviceNetwork.PropertiesExporter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Antilatency.DeviceNetwork.PropertiesExporter> ${CMAKE_CURRENT_SOURCE_DIR}/config.json $<TARGET_FILE_DIR:Antilatency.DeviceNetwork.PropertiesExporter>
    COMMENT "Copy default config.json"
)