# ==============================================================================
# DataNuke - Secure Data Deletion Tool
# Makes data powerless
#
# CMake Build Configuration for cross-platform compilation
# Supported platforms: Windows, Linux, macOS
# Requirements: CMake 3.15+, OpenSSL 1.1.0+ or 3.x
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(datanuke VERSION 1.0.0 LANGUAGES C)

# Use C11 standard for modern C features and security
# C11 provides: _Static_assert, anonymous structs/unions, aligned_alloc
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Export compile_commands.json for IDE integration and code analysis tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Build Configuration
# ==============================================================================

# Default to Debug build if not specified
# Debug: Full symbols, no optimization, assertions enabled
# Release: Full optimization, no debug info, assertions disabled
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Compiler flags for security and code quality
# Debug: -g (debug symbols), -O0 (no optimization for debugging)
#        -Wall -Wextra -Wpedantic (comprehensive warnings)
# Release: -O3 (aggressive optimization), -DNDEBUG (disable assertions)
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# ==============================================================================
# Platform Detection
# ==============================================================================

# Define platform-specific macros used in platform.c for conditional compilation
# PLATFORM_WINDOWS: Windows API (CreateFile, DeviceIoControl, VirtualLock)
# PLATFORM_MACOS:   macOS specific device handling
# PLATFORM_LINUX:   Linux ioctl and mlock/munlock
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# ==============================================================================
# Source Files and Build Target
# ==============================================================================

# Public API headers
include_directories(include)

# Core implementation files
# main.c:     CLI interface and BSI encryption workflow
# crypto.c:   AES-256 encryption and key management
# platform.c: Platform-specific device/memory operations
set(SOURCES
    src/main.c
    src/crypto.c
    src/platform.c
)

# Build datanuke executable
add_executable(datanuke ${SOURCES})

# ==============================================================================
# Dependencies and Linking
# ==============================================================================

# OpenSSL provides AES-256-CBC encryption and CSPRNG (RAND_bytes)
# Requires: OpenSSL 1.1.0+ or 3.x
# Links: libcrypto (EVP_*, RAND_*, ERR_* functions)
find_package(OpenSSL REQUIRED)
target_link_libraries(datanuke OpenSSL::Crypto)

# Platform-specific system libraries
# Linux: pthread for thread-safe OpenSSL operations
if(UNIX AND NOT APPLE)
    target_link_libraries(datanuke pthread)
endif()

# Installation
install(TARGETS datanuke DESTINATION bin)
