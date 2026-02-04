add_library(AntilatencySDK INTERFACE)

add_library(Antilatency::SDK ALIAS AntilatencySDK)

target_include_directories(AntilatencySDK INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/AntilatencySDK/Api
)

