#include "esp-bsp.h"
#include "esp_log.h"

static const char* TAG = "BSP";

esp_err_t bsp_init(void)
{
    ESP_LOGI(TAG, "初始化BSP");
    
    // 初始化播放器
    esp_err_t ret = bsp_extra_player_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "播放器初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "BSP初始化成功");
    return ESP_OK;
}