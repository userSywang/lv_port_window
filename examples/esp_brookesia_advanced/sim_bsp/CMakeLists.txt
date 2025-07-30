cmake_minimum_required(VERSION 3.12.4)


project(sim_bsp)

file(GLOB_RECURSE APPS_C_SRCS ./*.c)

add_library(${PROJECT_NAME} STATIC ${APPS_C_SRCS} )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bsp)