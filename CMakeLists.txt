cmake_minimum_required(VERSION 3.10)

project(Antilatency.PropertiesExporter)

set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR}/AntilatencySdk/Api)
add_executable(Antilatency.PropertiesExporter)

add_library(Application INTERFACE)

target_sources(Application INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(Antilatency.PropertiesExporter PUBLIC
    Application
    CLI11::CLI11
    Antilatency::SDK
)


add_subdirectory(Dependencies)
include(Dependencies/AntilatencySDK.Cmake/CopySDK.cmake)

copy_antilatency_sdk(Antilatency.PropertiesExporter)