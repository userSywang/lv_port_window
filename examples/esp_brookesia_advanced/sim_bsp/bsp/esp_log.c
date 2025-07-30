#include "esp_log.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static FILE *log_file = NULL;

void esp_log_init(const char *filename) {
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        printf("Failed to open log file\n");
    }
}

void esp_log_close() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

void esp_log_write(const char *tag, const char *level, const char *format, va_list args) {
    // 获取当前时间
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    // 打印到终端
    printf("[%s][%s][%s] ", time_str, level, tag);
    vprintf(format, args);
    printf("\n");

    // 打印到文件
    if (log_file != NULL) {
        fprintf(log_file, "[%s][%s][%s] ", time_str, level, tag);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

void esp_log_output(const char *tag, const char *level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    esp_log_write(tag, level, format, args);
    va_end(args);
}

void esp_log_level_set(const char *tag, esp_log_level_t level) {
    // 设置日志级别（暂不实现具体逻辑）
}

