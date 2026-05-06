#ifndef SCROLL_TEST_SCROLL_BAR_H
#define SCROLL_TEST_SCROLL_BAR_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scroll_bar_s {
    lv_obj_t *content;
    lv_timer_t *timer;
    uint16_t child_nums;
    int16_t spacing;
    uint16_t selected;
    uint16_t direction;
    uint16_t onesnap;
    uint16_t optimized;
    uint16_t infinity_loop;
    uint16_t edge_indicator;
    lv_coord_t last_scroll_pos;
    float scale_min;
    float scale_max;
    const char **item_titles;
    struct {
        uint16_t width;
        uint16_t height;
    } item_size;
    lv_obj_t **children;
    lv_obj_t **title_labels;
    lv_obj_t *edge_bar;
    lv_timer_t *edge_timer;
    void (*item_childs_create_cb)(lv_obj_t *item, uint16_t index);
    void (*item_childs_scale_cb)(lv_obj_t *item, float scale);
    void (*item_childs_click_cb)(lv_obj_t *item, uint16_t index);
} scroll_bar_s;

lv_obj_t *scroll_bar_create(lv_obj_t *parent, struct scroll_bar_s *scroll_bar);
void scroll_bar_refresh(struct scroll_bar_s *scroll_bar);
void scroll_bar_show_edge_indicator(struct scroll_bar_s *scroll_bar, uint8_t edge_type);

#ifdef __cplusplus
}
#endif

#endif
