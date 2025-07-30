#pragma once

#include "esp_log.h"
#include "nvs.h"
#include "bsp_board_extra.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化BSP
 *
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t bsp_init(void);

#ifdef __cplusplus
}
#endif