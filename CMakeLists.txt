cmake_minimum_required(VERSION 3.12.4)

project(lv_port_sdl)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/out/lv_port_sdl)

# 添加驱动编译选项
option(ENABLE_DRIVERS "Enable building drivers" OFF)

set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "Path defined for lv_conf.h")

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)        # C11
set(CMAKE_CXX_STANDARD 17)      # C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set compiler flags
set(CMAKE_C_FLAGS "-O0 -g -Wall -Wextra -Wshadow -Wundef -Wno-unused-function")
set(CMAKE_CXX_FLAGS "-O0 -g -Wall -Wextra -Wshadow -Wundef -Wno-unused-function -Wno-unused-parameter \
                    -Wno-missing-field-initializers")
# Set cmake options
set(CMAKE_OBJECT_PATH_MAX 512)  # Avoid long path error in Windows
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(ENABLE_DRIVERS)
    add_subdirectory(drivers)
endif()

add_subdirectory(lv_drivers EXCLUDE_FROM_ALL)
add_subdirectory(lvgl EXCLUDE_FROM_ALL)
add_subdirectory(esp-brookesia)
add_subdirectory(examples)