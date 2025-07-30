#pragma once

#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 文件迭代器实例类型
 */
typedef struct {
    char* directory;     /*!< 目录路径 */
    char** file_list;    /*!< 文件列表 */
    int file_count;      /*!< 文件数量 */
    int current_index;   /*!< 当前索引 */
} file_iterator_instance_t;

uint32_t file_iterator_get_count(file_iterator_instance_t *file_iterator);

// 根据索引获取音乐文件名
const char *file_iterator_get_name_from_index(uint32_t index, uint32_t id);

// 获取当前音乐文件索引
uint32_t file_iterator_get_index(file_iterator_instance_t *file_iterator);

// 设置当前音乐文件索引
void file_iterator_set_index(uint32_t index,uint32_t id);


#ifdef __cplusplus
}
#endif