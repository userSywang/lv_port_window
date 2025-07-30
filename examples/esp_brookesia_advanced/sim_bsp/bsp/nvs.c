#include "nvs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 简单的键值存储模拟
typedef struct {
    char key[32];
    int32_t value;
} nvs_entry_t;

typedef struct {
    char namespace[32];
    nvs_entry_t* entries;
    size_t entry_count;
    nvs_open_mode_t mode;
} nvs_storage_t;

// 全局存储
static nvs_storage_t* g_storages = NULL;
static size_t g_storage_count = 0;

esp_err_t nvs_open(const char* name, nvs_open_mode_t open_mode, nvs_handle_t* out_handle)
{
    if (name == NULL || out_handle == NULL) {
        return ESP_ERR_NVS_BASE;
    }

    // 检查是否已存在该命名空间
    for (size_t i = 0; i < g_storage_count; i++) {
        if (strcmp(g_storages[i].namespace, name) == 0) {
            *out_handle = (nvs_handle_t)i;
            return ESP_OK;
        }
    }

    // 创建新的存储
    g_storage_count++;
    g_storages = realloc(g_storages, g_storage_count * sizeof(nvs_storage_t));
    if (g_storages == NULL) {
        return ESP_ERR_NVS_BASE;
    }

    nvs_storage_t* storage = &g_storages[g_storage_count - 1];
    strncpy(storage->namespace, name, sizeof(storage->namespace) - 1);
    storage->namespace[sizeof(storage->namespace) - 1] = '\0';
    storage->entries = NULL;
    storage->entry_count = 0;
    storage->mode = open_mode;

    *out_handle = (nvs_handle_t)(g_storage_count - 1);
    return ESP_OK;
}

esp_err_t nvs_set_i32(nvs_handle_t handle, const char* key, int32_t value)
{
    if (handle >= g_storage_count || key == NULL) {
        return ESP_ERR_NVS_BASE;
    }

    nvs_storage_t* storage = &g_storages[handle];
    
    // 只读模式不允许写入
    if (storage->mode == NVS_READONLY) {
        return ESP_ERR_NVS_BASE;
    }

    // 检查是否已存在该键
    for (size_t i = 0; i < storage->entry_count; i++) {
        if (strcmp(storage->entries[i].key, key) == 0) {
            storage->entries[i].value = value;
            return ESP_OK;
        }
    }

    // 创建新的条目
    storage->entry_count++;
    storage->entries = realloc(storage->entries, storage->entry_count * sizeof(nvs_entry_t));
    if (storage->entries == NULL) {
        return ESP_ERR_NVS_BASE;
    }

    nvs_entry_t* entry = &storage->entries[storage->entry_count - 1];
    strncpy(entry->key, key, sizeof(entry->key) - 1);
    entry->key[sizeof(entry->key) - 1] = '\0';
    entry->value = value;

    return ESP_OK;
}

esp_err_t nvs_get_i32(nvs_handle_t handle, const char* key, int32_t* out_value)
{
    if (handle >= g_storage_count || key == NULL || out_value == NULL) {
        return ESP_ERR_NVS_BASE;
    }

    nvs_storage_t* storage = &g_storages[handle];

    // 检查是否存在该键
    for (size_t i = 0; i < storage->entry_count; i++) {
        if (strcmp(storage->entries[i].key, key) == 0) {
            *out_value = storage->entries[i].value;
            return ESP_OK;
        }
    }

    return ESP_ERR_NVS_NOT_FOUND;
}

const char* esp_err_to_name(esp_err_t code)
{
    switch (code) {
        case ESP_OK:
            return "ESP_OK";
        case ESP_ERR_NVS_NOT_FOUND:
            return "ESP_ERR_NVS_NOT_FOUND";
        default:
            return "UNKNOWN_ERROR";
    }
}