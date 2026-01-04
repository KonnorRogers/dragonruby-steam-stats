cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "steam_stats")
project(${PROJECT_NAME} CXX)

message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

# Detect platform
set(STEAMWORKS_SDK "${CMAKE_SOURCE_DIR}/steam-sdk")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check for cross-compilation environment variable
if(DEFINED ENV{TARGET_ARCH})
    if(ENV{TARGET_ARCH} STREQUAL "arm64")
        set(TARGET_ARCH $ENV{TARGET_ARCH})
        message(STATUS "Cross-compiling for architecture: ${TARGET_ARCH}")
        set(CMAKE_SYSTEM_NAME Linux)
        set(CMAKE_SYSTEM_PROCESSOR aarch64)
        set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
        set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
        # Force CMake to recognize this as cross-compilation
        set(CMAKE_CROSSCOMPILING TRUE)
    endif()
endif()


if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(PLATFORM "windows-amd64")
    set(DLLEXT "dll")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin/win64")
        set(STEAM_LIB "steam_api64")
        set(STEAM_DYLIB "steam_api64.dll")
    else()
        set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin")
        set(STEAM_LIB "steam_api")
        set(STEAM_DYLIB "steam_api.dll")
    endif()
elseif(APPLE)
    set(PLATFORM "macos")
    set(DLLEXT "dylib")
    set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin/osx")
    set(STEAM_LIB "steam_api")
    set(STEAM_DYLIB "libsteam_api.dylib")
elseif(UNIX)
    set(DLLEXT "so")

    # Check TARGET_ARCH first (for cross-compilation), then CMAKE_SYSTEM_PROCESSOR
    if(DEFINED TARGET_ARCH AND TARGET_ARCH STREQUAL "arm64")
        set(PLATFORM "linux-arm64")
        set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin/linux64")
    else()
        set(PLATFORM "linux-amd64")
        set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin/linux64")
    endif()

    set(STEAM_LIB "steam_api")
    set(STEAM_DYLIB "libsteam_api.so")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Directories
set(SOURCE_FILE "${CMAKE_SOURCE_DIR}/src/steam_stats.cpp")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/mygame/native/${PLATFORM}")

# Set output directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# Create shared library
add_library(steam_stats SHARED ${SOURCE_FILE})
target_include_directories(steam_stats PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${STEAMWORKS_SDK}/public
)

# Set C++ standard
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# Set library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".${DLLEXT}"
    POSITION_INDEPENDENT_CODE ON
)

# Link Steam API
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${STEAM_LIB_DIR}/${STEAM_LIB}.lib)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${STEAM_LIB_DIR}/${STEAM_DYLIB})
endif()

# Set rpath for runtime library loading
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
elseif(UNIX AND NOT WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Copy Steam dylib to output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${STEAM_LIB_DIR}/${STEAM_DYLIB}"
        "${BUILD_DIR}/${STEAM_DYLIB}"
    COMMENT "Copying ${STEAM_DYLIB} to ${BUILD_DIR}/"
)

# Custom clean target to remove native directory
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/mygame/native"
    COMMENT "Removing mygame/native directory"
)
