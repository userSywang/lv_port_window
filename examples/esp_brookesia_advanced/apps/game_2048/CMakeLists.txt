cmake_minimum_required(VERSION 3.12.4)


project(game_2048)

file(GLOB_RECURSE APPS_C_SRCS ./*.c)
file(GLOB_RECURSE APPS_CPP_SRCS ./*.cpp)

add_library(${PROJECT_NAME} STATIC ${APPS_C_SRCS} ${APPS_CPP_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC sim_bsp lvgl esp-brookesia)

target_compile_options(
    ${PROJECT_NAME} 
    PRIVATE
    $<$<COMPILE_LANGUAGE:C>:
        -Wno-format
        -Wno-incompatible-pointer-types
        -Wno-int-conversion
    >
    $<$<COMPILE_LANGUAGE:CXX>:
        -Wno-format
        -Wno-conversion-null
        -Wno-attributes
        -Wno-write-strings
    >
    -DLV_LVGL_H_INCLUDE_SIMPLE
)