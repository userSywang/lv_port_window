#pragma once

#include <stdbool.h>
#include "nvs.h"
#include "audio_player.h"
#include "file_iterator.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif


#define BSP_SD_MOUNT_POINT "./spiffs"

/**
 * @brief SPIFFS挂载点
 */
#define BSP_SPIFFS_MOUNT_POINT "./spiffs"

/**
 * @brief 初始化播放器
 *
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t bsp_extra_player_init(void);

/**
 * @brief 播放指定文件
 *
 * @param file_path 文件路径
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t bsp_extra_player_play_file(const char* file_path);

/**
 * @brief 初始化文件迭代器实例
 *
 * @param directory 目录路径
 * @param iterator 输出参数，将包含文件迭代器实例
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t bsp_extra_file_instance_init(const char* directory, file_iterator_instance_t** iterator);

/**
 * @brief 获取下一个文件
 *
 * @param iterator 文件迭代器实例
 * @param file_path 输出参数，将包含文件路径
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t bsp_extra_file_get_next(file_iterator_instance_t* iterator, char** file_path);

/**
 * @brief 销毁文件迭代器实例
 *
 * @param iterator 文件迭代器实例
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t bsp_extra_file_instance_destroy(file_iterator_instance_t* iterator);

bool bsp_extra_player_is_playing_by_index(uint32_t index, uint32_t id);

// 恢复音乐播放
void audio_player_resume(void);

// 播放指定索引的音乐
void bsp_extra_player_play_index(uint32_t index, uint32_t id);



/*
 * @brief 显示解锁 
 */
void bsp_display_unlock(void);

/*
 * @brief 显示锁定
 */
void bsp_display_lock(int us);

#ifdef __cplusplus
}
#endif