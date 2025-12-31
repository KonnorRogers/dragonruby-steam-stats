cmake_minimum_required(VERSION 3.10)
project(extension C CXX)

# Detect platform
set(STEAMWORKS_SDK "${CMAKE_SOURCE_DIR}/steam-sdk")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    set(PLATFORM "macos")
    set(DLLEXT "dylib")
    set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin/osx")
    set(STEAM_LIB "steam_api")
    set(STEAM_DYLIB "libsteam_api.dylib")
elseif(UNIX)
    set(PLATFORM "linux-amd64")
    set(DLLEXT "so")
    set(STEAM_LIB_DIR "${STEAMWORKS_SDK}/redistributable_bin/linux64")
    set(STEAM_LIB "steam_api")
    set(STEAM_DYLIB "libsteam_api.so")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Directories
set(SOURCE_FILE "${CMAKE_SOURCE_DIR}/lib/extension.cpp")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/mygame/native/${PLATFORM}")

# Set output directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# Create shared library
add_library(extension SHARED ${SOURCE_FILE})

target_include_directories(extension PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${STEAMWORKS_SDK}/public
)

# Set C++ standard
set_property(TARGET extension PROPERTY CXX_STANDARD 11)

# Set library properties
set_target_properties(extension PROPERTIES
    PREFIX ""
    SUFFIX ".${DLLEXT}"
    POSITION_INDEPENDENT_CODE ON
)

# Link Steam API
target_link_libraries(extension PRIVATE ${STEAM_LIB_DIR}/${STEAM_DYLIB})

# Set rpath for runtime library loading
if(APPLE)
    set_target_properties(extension PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
elseif(UNIX)
    set_target_properties(extension PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Copy Steam dylib to output directory
add_custom_command(TARGET extension POST_BUILD
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
