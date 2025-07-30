#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "file_iterator.h"

// 模拟音乐文件迭代器
static uint32_t current_index = 0;
static const char *music_files[] = {"track1.mp3", "track2.mp3", "track3.mp3"};
static const uint32_t music_file_count = 3;


// 获取音乐文件数量
uint32_t file_iterator_get_count(file_iterator_instance_t *file_iterator) {
    return music_file_count;
}

// 根据索引获取音乐文件名
const char *file_iterator_get_name_from_index(uint32_t index,uint32_t id) {
    if (index >= music_file_count) return NULL;
    return music_files[index];
}

// 获取当前音乐文件索引
uint32_t file_iterator_get_index(file_iterator_instance_t *file_iterator) {
    return current_index;
}


void file_iterator_set_index(uint32_t index,uint32_t id) {
    if (index < music_file_count) current_index = index;
}


