#include "esp_log.h"
#include <stdio.h>
#include <string.h>

// 默认日志级别
static esp_log_level_t s_log_level = ESP_LOG_INFO;

void esp_log_level_set(const char* tag, esp_log_level_t level)
{
    // 在模拟环境中，我们简单地设置全局日志级别
    s_log_level = level;
    (void)tag; // 避免未使用参数警告
}