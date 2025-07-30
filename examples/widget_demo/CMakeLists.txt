cmake_minimum_required(VERSION 3.12.4)


project(widget_demo)

# 添加驱动编译选项
option(ENABLE_DRIVERS "Enable building drivers" OFF)

set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "Path defined for lv_conf.h")

# 查找SDL2库
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)

# 链接 LVGL 库
target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)

# 链接 lv_drivers 库
target_link_libraries(${PROJECT_NAME} PRIVATE lv_drivers)

target_link_libraries(${PROJECT_NAME} PRIVATE lvgl_demos)

# 链接 SDL2 库
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_STATIC_LIBRARIES})

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)