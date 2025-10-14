# cmake_minimum_required(VERSION 3.12.4)

# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# if(NOT ESP_PLATFORM)
#   project(lvgl LANGUAGES C HOMEPAGE_URL https://github.com/lvgl/lvgl)
#   if(NOT (CMAKE_C_COMPILER_ID STREQUAL "MSVC"))
#     enable_language(CXX ASM)
#   else()
#     enable_language(CXX)
#   endif()
# endif()

# set(LVGL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# if(ESP_PLATFORM)
#   include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/esp.cmake)
# elseif(ZEPHYR_BASE)
#   include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/zephyr.cmake)
# elseif(MICROPY_DIR)
#   include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/micropython.cmake)
# else()
#   include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/os_desktop.cmake)
# endif()

# #[[
#     unfortunately CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS does not work for global data. 
#     for global data we still need decl specs.
#     Check out the docs to learn more about the limitations of CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
#     https://cmake.org/cmake/help/latest/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html#prop_tgt:WINDOWS_EXPORT_ALL_SYMBOLS

#     For all compiled sources within the library (i.e. basically all lvgl files) we need to use dllexport.
#     For all compiled sources from outside the library (i.e. files which include lvgl headers) we need to use dllimport.
#     We can do this by using CMakes INTERFACE and PRIVATE keyword.
#   ]]
# if (MSVC)
#   target_compile_definitions(lvgl
#     INTERFACE LV_ATTRIBUTE_EXTERN_DATA=__declspec\(dllimport\)
#     PRIVATE LV_ATTRIBUTE_EXTERN_DATA=__declspec\(dllexport\)
#   )
# endif()

# 添加LVGL核心源文件
file(GLOB_RECURSE LVGL_SOURCES
    "src/*.c"
    "src/core/*.c"
    "src/display/*.c" 
    "src/indev/*.c"
    "src/misc/*.c"
    "src/widgets/*.c"
    "src/font/*.c"
    "src/draw/*.c"
    "src/themes/*.c"
)

# 排除不需要的目录
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*/examples/.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*/tests/.*")

idf_component_register(
    SRCS 
        ${LVGL_SOURCES}
    INCLUDE_DIRS 
        "src"
        "."
    REQUIRES 
        freertos
    PRIV_REQUIRES 
        freertos
)
