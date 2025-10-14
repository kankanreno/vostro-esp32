# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Include the base project.cmake from ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set supported targets
set(SUPPORTED_TARGETS esp32s3)

# 添加ESP-IDF组件和自定义组件
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components ./components)

# 启用compile_commands.json生成
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define project name
project(lvgl)
