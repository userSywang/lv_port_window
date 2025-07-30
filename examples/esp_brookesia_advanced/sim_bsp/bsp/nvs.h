#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NVS错误代码
 */
typedef enum {
    ESP_OK = 0,                    /*!< 操作成功 */
    ESP_ERR_NVS_NOT_FOUND = 0x1100, /*!< 未找到请求的键 */
    ESP_ERR_NVS_BASE = 0x1000      /*!< 起始错误代码 */
} esp_err_t;

/**
 * @brief NVS句柄类型
 */
typedef uint32_t nvs_handle_t;

/**
 * @brief NVS打开模式
 */
typedef enum {
    NVS_READONLY = 0x01,  /*!< 只读模式 */
    NVS_READWRITE = 0x02  /*!< 读写模式 */
} nvs_open_mode_t;

/**
 * @brief 打开非易失性存储命名空间
 *
 * @param name 命名空间名称
 * @param open_mode 打开模式（NVS_READONLY或NVS_READWRITE）
 * @param out_handle 输出参数，将包含NVS句柄
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t nvs_open(const char* name, nvs_open_mode_t open_mode, nvs_handle_t* out_handle);

/**
 * @brief 设置整数值
 *
 * @param handle NVS句柄
 * @param key 键名
 * @param value 要设置的值
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t nvs_set_i32(nvs_handle_t handle, const char* key, int32_t value);

/**
 * @brief 获取整数值
 *
 * @param handle NVS句柄
 * @param key 键名
 * @param out_value 输出参数，将包含读取的值
 * @return esp_err_t ESP_OK成功，否则失败
 */
esp_err_t nvs_get_i32(nvs_handle_t handle, const char* key, int32_t* out_value);

/**
 * @brief 将错误代码转换为字符串
 *
 * @param code 错误代码
 * @return const char* 错误描述字符串
 */
const char* esp_err_to_name(esp_err_t code);

#ifdef __cplusplus
}
#endif