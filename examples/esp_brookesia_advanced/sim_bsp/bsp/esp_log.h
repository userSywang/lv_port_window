#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 日志级别
 */
typedef enum {
    ESP_LOG_NONE,       /*!< 无日志输出 */
    ESP_LOG_ERROR,      /*!< 错误日志 */
    ESP_LOG_WARN,       /*!< 警告日志 */
    ESP_LOG_INFO,       /*!< 信息日志 */
    ESP_LOG_DEBUG,      /*!< 调试日志 */
    ESP_LOG_VERBOSE     /*!< 详细日志 */
} esp_log_level_t;

/**
 * @brief 设置日志级别
 *
 * @param tag 日志标签
 * @param level 日志级别
 */
void esp_log_level_set(const char* tag, esp_log_level_t level);

/**
 * @brief 错误级别日志宏
 */
#define ESP_LOGE(tag, format, ...) printf("[E][%s] " format "\n", tag, ##__VA_ARGS__)

/**
 * @brief 警告级别日志宏
 */
#define ESP_LOGW(tag, format, ...) printf("[W][%s] " format "\n", tag, ##__VA_ARGS__)

/**
 * @brief 信息级别日志宏
 */
#define ESP_LOGI(tag, format, ...) printf("[I][%s] " format "\n", tag, ##__VA_ARGS__)

/**
 * @brief 调试级别日志宏
 */
#define ESP_LOGD(tag, format, ...) printf("[D][%s] " format "\n", tag, ##__VA_ARGS__)

/**
 * @brief 详细级别日志宏
 */
#define ESP_LOGV(tag, format, ...) printf("[V][%s] " format "\n", tag, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif