cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(SUPPORTED_TARGETS esp32s3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(dht22)
