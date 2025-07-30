#include "bsp_board_extra.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static const char* TAG = "BSP_EXTRA";

esp_err_t bsp_extra_player_init(void)
{
    ESP_LOGI(TAG, "初始化播放器");
    // 在模拟环境中，我们只需返回成功
    return ESP_OK;
}

esp_err_t bsp_extra_player_play_file(const char* file_path)
{
    if (file_path == NULL) {
        ESP_LOGE(TAG, "文件路径为空");
        return ESP_ERR_NVS_BASE;
    }

    ESP_LOGI(TAG, "播放文件: %s", file_path);
    // 在模拟环境中，我们只需打印文件名
    return ESP_OK;
}

esp_err_t bsp_extra_file_instance_init(const char* directory, file_iterator_instance_t** iterator)
{
    if (directory == NULL || iterator == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_NVS_BASE;
    }

    // 分配迭代器内存
    *iterator = (file_iterator_instance_t*)malloc(sizeof(file_iterator_instance_t));
    if (*iterator == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NVS_BASE;
    }

    // 初始化迭代器
    (*iterator)->directory = strdup(directory);
    (*iterator)->file_list = NULL;
    (*iterator)->file_count = 0;
    (*iterator)->current_index = 0;

    // 打开目录
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        ESP_LOGE(TAG, "无法打开目录: %s", directory);
        free((*iterator)->directory);
        free(*iterator);
        *iterator = NULL;
        return ESP_ERR_NVS_BASE;
    }

    // 计算文件数量
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // 只计算常规文件
            (*iterator)->file_count++;
        }
    }

    // 如果没有文件，返回
    if ((*iterator)->file_count == 0) {
        ESP_LOGI(TAG, "目录为空: %s", directory);
        closedir(dir);
        return ESP_OK;
    }

    // 分配文件列表内存
    (*iterator)->file_list = (char**)malloc((*iterator)->file_count * sizeof(char*));
    if ((*iterator)->file_list == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        free((*iterator)->directory);
        free(*iterator);
        *iterator = NULL;
        closedir(dir);
        return ESP_ERR_NVS_BASE;
    }

    // 重新打开目录
    rewinddir(dir);

    // 填充文件列表
    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < (*iterator)->file_count) {
        if (entry->d_type == DT_REG) { // 只添加常规文件
            // 构建完整路径
            size_t path_len = strlen(directory) + strlen(entry->d_name) + 2; // +2 for '/' and '\0'
            char* full_path = (char*)malloc(path_len);
            if (full_path == NULL) {
                ESP_LOGE(TAG, "内存分配失败");
                // 清理已分配的内存
                for (int i = 0; i < index; i++) {
                    free((*iterator)->file_list[i]);
                }
                free((*iterator)->file_list);
                free((*iterator)->directory);
                free(*iterator);
                *iterator = NULL;
                closedir(dir);
                return ESP_ERR_NVS_BASE;
            }

            snprintf(full_path, path_len, "%s/%s", directory, entry->d_name);
            (*iterator)->file_list[index++] = full_path;
        }
    }

    closedir(dir);
    ESP_LOGI(TAG, "文件迭代器初始化成功，找到 %d 个文件", (*iterator)->file_count);
    return ESP_OK;
}

esp_err_t bsp_extra_file_get_next(file_iterator_instance_t* iterator, char** file_path)
{
    if (iterator == NULL || file_path == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_NVS_BASE;
    }

    // 检查是否有文件
    if (iterator->file_count == 0) {
        ESP_LOGI(TAG, "没有文件");
        *file_path = NULL;
        return ESP_ERR_NVS_NOT_FOUND;
    }

    // 检查是否已经遍历完所有文件
    if (iterator->current_index >= iterator->file_count) {
        // 循环回到第一个文件
        iterator->current_index = 0;
    }

    // 获取当前文件路径
    *file_path = iterator->file_list[iterator->current_index];
    
    // 移动到下一个文件
    iterator->current_index++;

    ESP_LOGI(TAG, "获取下一个文件: %s", *file_path);
    return ESP_OK;
}

esp_err_t bsp_extra_file_instance_destroy(file_iterator_instance_t* iterator)
{
    if (iterator == NULL) {
        ESP_LOGE(TAG, "迭代器为空");
        return ESP_ERR_NVS_BASE;
    }

    // 释放文件路径
    for (int i = 0; i < iterator->file_count; i++) {
        free(iterator->file_list[i]);
    }

    // 释放文件列表
    free(iterator->file_list);
    
    // 释放目录路径
    free(iterator->directory);
    
    // 释放迭代器
    free(iterator);

    ESP_LOGI(TAG, "文件迭代器销毁成功");
    return ESP_OK;
}

static uint32_t current_index = 0;
static const uint32_t music_file_count = 3;
// 检查指定索引的音乐是否正在播放
bool bsp_extra_player_is_playing_by_index(uint32_t index, uint32_t id) {
    return (index == current_index);
}

// 恢复音乐播放
void audio_player_resume(void) {
    // 模拟恢复播放
}

// 播放指定索引的音乐
void bsp_extra_player_play_index(uint32_t index,uint32_t id) {
    if (index < music_file_count) current_index = index;
}

void bsp_display_unlock(void)
{

}


void bsp_display_lock(int us)
{

}