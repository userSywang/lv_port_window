cmake_minimum_required(VERSION 3.12.4)


project(esp_brookesia_advanced)

# 添加驱动编译选项
option(ENABLE_DRIVERS "Enable building drivers" OFF)

set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "Path defined for lv_conf.h")

# 查找SDL2库
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


add_subdirectory(sim_bsp)
add_subdirectory(apps)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)

# 链接 LVGL 
target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)

# 链接 lv_drivers 
target_link_libraries(${PROJECT_NAME} PRIVATE lv_drivers)

target_link_libraries(${PROJECT_NAME} PRIVATE esp-brookesia)

# 动态链接 SDL2 
#target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})

# 静态链接 SDL2 
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_STATIC_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

target_link_libraries(${PROJECT_NAME} PRIVATE game_2048)
target_link_libraries(${PROJECT_NAME} PRIVATE calculator)
target_link_libraries(${PROJECT_NAME} PRIVATE draw)
target_link_libraries(${PROJECT_NAME} PRIVATE music_player)
target_link_libraries(${PROJECT_NAME} PRIVATE video_player)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/spiffs DESTINATION bin)