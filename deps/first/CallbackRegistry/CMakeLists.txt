cmake_minimum_required(VERSION 3.22)

set(TARGET CallbackRegistry)
project(${TARGET})
message("Project: ${TARGET} (HEADER-ONLY)")

add_library(${TARGET} INTERFACE)

# Enabling c++23 support
target_compile_features(${TARGET} INTERFACE cxx_std_23)

target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Make headers visible in the IDE
make_headers_visible(${TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/include")