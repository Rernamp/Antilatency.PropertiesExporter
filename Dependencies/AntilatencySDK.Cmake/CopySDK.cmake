function(copy_antilatency_sdk target_name)

    if(WIN32)
        file(GLOB ANTILATENCY_LIBS ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/AntilatencySDK/Bin/WindowsDesktop/x64/*.dll)
    endif(WIN32)
    if(UNIX)
        target_link_libraries(Antilatency.PropertiesExporter dl stdc++fs pthread)

        EXECUTE_PROCESS(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
        message(STATUS "Architecture: ${ARCHITECTURE}")

        if(${ARCHITECTURE} STREQUAL "aarch64")
            file(GLOB ANTILATENCY_LIBS ${CMAKE_CURRENT_FUNCTION_LIST_DIR }/AntilatencySDK/Bin/Linux/aarch64_linux_gnu/*.so)
        elseif(${ARCHITECTURE} STREQUAL "armv7l")
            file(GLOB ANTILATENCY_LIBS ${CMAKE_CURRENT_FUNCTION_LIST_DIR }/AntilatencySDK/Bin/Linux/arm_linux_gnueabihf/*.so)
        elseif(${ARCHITECTURE} STREQUAL "x86_64")
            file(GLOB ANTILATENCY_LIBS ${CMAKE_CURRENT_FUNCTION_LIST_DIR }/AntilatencySDK/Bin/Linux/x86_64/*.so)
        endif()

    endif(UNIX)

    if(${ANTILATENCY_LIBS} STREQUAL "")
        message(SEND_ERROR "Failed to find Antilatency libs")
    endif()

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ANTILATENCY_LIBS} $<TARGET_FILE_DIR:${target_name}>)
endfunction()

