#include "scroll_bar.h"

#include <stdlib.h>

#define EDGE_INDICATOR_WIDTH 6
#define EDGE_INDICATOR_COLOR lv_color_hex(0x00FF00)

typedef enum {
    EDGE_NONE = 0,
    EDGE_LEFT = 1,
    EDGE_RIGHT = 2,
    EDGE_TOP = 3,
    EDGE_BOTTOM = 4
} edge_type_t;

static void scroll_bar_scroll_cb(lv_event_t *e);
static void scroll_bar_destroy_cb(lv_event_t *e);
static void scroll_bar_scroll_end_cb(lv_event_t *e);
static void scroll_bar_timer_cb(lv_timer_t *timer);
static void edge_indicator_timer_cb(lv_timer_t *timer);
static void scroll_bar_child_click_cb(lv_event_t *e);

#if defined(lv_timer_get_user_data)
#define MY_LV_TIMER_GET_USER_DATA(t) lv_timer_get_user_data(t)
#else
#define MY_LV_TIMER_GET_USER_DATA(t) ((t)->user_data)
#endif

#if defined(lv_timer_get_paused)
#define MY_LV_TIMER_GET_PAUSED(t) lv_timer_get_paused(t)
#else
#define MY_LV_TIMER_GET_PAUSED(t) ((t)->paused)
#endif

static void scroll_bar_child_click_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    struct scroll_bar_s *scroll_bar = (struct scroll_bar_s *)lv_event_get_user_data(e);

    if((code != LV_EVENT_CLICKED) || (scroll_bar == NULL) || (scroll_bar->children == NULL) ||
       (scroll_bar->item_childs_click_cb == NULL)) {
        return;
    }

    for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
        if(scroll_bar->children[i] == obj) {
            scroll_bar->item_childs_click_cb(obj, i);
            break;
        }
    }
}

static void scroll_bar_timer_cb(lv_timer_t *timer)
{
    struct scroll_bar_s *scroll_bar = (struct scroll_bar_s *)MY_LV_TIMER_GET_USER_DATA(timer);
    if((scroll_bar == NULL) || (scroll_bar->children == NULL) || (scroll_bar->selected >= scroll_bar->child_nums)) {
        return;
    }

    lv_obj_scroll_to_view(scroll_bar->children[scroll_bar->selected], LV_ANIM_ON);
    if(scroll_bar->timer) {
        lv_timer_pause(scroll_bar->timer);
    }
}

static void scroll_bar_scroll_end_cb(lv_event_t *e)
{
    struct scroll_bar_s *scroll_bar = lv_event_get_user_data(e);
    if(scroll_bar && scroll_bar->timer) {
        lv_timer_reset(scroll_bar->timer);
        lv_timer_resume(scroll_bar->timer);
    }
}

static void edge_indicator_timer_cb(lv_timer_t *timer)
{
    struct scroll_bar_s *scroll_bar = (struct scroll_bar_s *)MY_LV_TIMER_GET_USER_DATA(timer);
    if(scroll_bar && scroll_bar->edge_bar) {
        lv_obj_add_flag(scroll_bar->edge_bar, LV_OBJ_FLAG_HIDDEN);
    }
}

void scroll_bar_show_edge_indicator(struct scroll_bar_s *scroll_bar, uint8_t edge_type)
{
    if((scroll_bar == NULL) || !scroll_bar->edge_indicator || (scroll_bar->edge_bar == NULL)) {
        return;
    }

    lv_obj_clear_flag(scroll_bar->edge_bar, LV_OBJ_FLAG_HIDDEN);

    if(scroll_bar->direction == 0) {
        if(edge_type == EDGE_LEFT) {
            lv_obj_align(scroll_bar->edge_bar, LV_ALIGN_LEFT_MID, 0, 0);
        } else if(edge_type == EDGE_RIGHT) {
            lv_obj_align(scroll_bar->edge_bar, LV_ALIGN_RIGHT_MID, 0, 0);
        }
        lv_obj_set_size(scroll_bar->edge_bar, EDGE_INDICATOR_WIDTH, LV_PCT(80));
        lv_obj_set_style_bg_grad_dir(scroll_bar->edge_bar, LV_GRAD_DIR_VER, 0);
    } else {
        if(edge_type == EDGE_TOP) {
            lv_obj_align(scroll_bar->edge_bar, LV_ALIGN_TOP_MID, 0, 0);
        } else if(edge_type == EDGE_BOTTOM) {
            lv_obj_align(scroll_bar->edge_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        lv_obj_set_size(scroll_bar->edge_bar, LV_PCT(80), EDGE_INDICATOR_WIDTH);
        lv_obj_set_style_bg_grad_dir(scroll_bar->edge_bar, LV_GRAD_DIR_HOR, 0);
    }

    lv_obj_set_style_bg_color(scroll_bar->edge_bar, EDGE_INDICATOR_COLOR, 0);
    lv_obj_set_style_bg_grad_color(scroll_bar->edge_bar, lv_color_hex(0x00AA00), 0);
    lv_obj_set_style_bg_opa(scroll_bar->edge_bar, LV_OPA_70, 0);
    lv_obj_set_style_radius(scroll_bar->edge_bar, 3, 0);

    if(scroll_bar->edge_timer) {
        lv_timer_reset(scroll_bar->edge_timer);
        lv_timer_resume(scroll_bar->edge_timer);
    }
}

lv_obj_t *scroll_bar_create(lv_obj_t *parent, struct scroll_bar_s *scroll_bar)
{
    int src_idx = 0;

    if((parent == NULL) || (scroll_bar == NULL) || (scroll_bar->child_nums == 0) || (scroll_bar->item_childs_create_cb == NULL)) {
        return NULL;
    }

    scroll_bar->content = lv_obj_create(parent);
    lv_obj_remove_style_all(scroll_bar->content);
    lv_obj_set_size(scroll_bar->content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scroll_snap_x(scroll_bar->content, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scroll_snap_y(scroll_bar->content, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(scroll_bar->content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(scroll_bar->content, scroll_bar->direction == 0 ? LV_DIR_HOR : LV_DIR_VER);

    if(scroll_bar->onesnap == 1) {
        lv_obj_add_flag(scroll_bar->content, LV_OBJ_FLAG_SCROLL_ONE);
    }

    scroll_bar->children = (lv_obj_t **)malloc(sizeof(lv_obj_t *) * scroll_bar->child_nums);
    if(scroll_bar->children == NULL) {
        return NULL;
    }

    if(scroll_bar->edge_indicator) {
        scroll_bar->edge_bar = lv_obj_create(parent);
        lv_obj_remove_style_all(scroll_bar->edge_bar);
        lv_obj_add_flag(scroll_bar->edge_bar, LV_OBJ_FLAG_HIDDEN);
        scroll_bar->edge_timer = lv_timer_create(edge_indicator_timer_cb, 1000, scroll_bar);
        if(scroll_bar->edge_timer) {
            lv_timer_pause(scroll_bar->edge_timer);
        }
    } else {
        scroll_bar->edge_bar = NULL;
        scroll_bar->edge_timer = NULL;
    }

    scroll_bar->timer = lv_timer_create(scroll_bar_timer_cb, 300, scroll_bar);
    if(scroll_bar->timer) {
        lv_timer_set_repeat_count(scroll_bar->timer, -1);
        lv_timer_pause(scroll_bar->timer);
    }
    scroll_bar->last_scroll_pos = 0;

    src_idx = (scroll_bar->selected >= scroll_bar->child_nums) ? (scroll_bar->child_nums - 1) : scroll_bar->selected;

    for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
        scroll_bar->children[i] = lv_obj_create(scroll_bar->content);
        scroll_bar->item_childs_create_cb(scroll_bar->children[i], i);
        lv_obj_clear_flag(scroll_bar->children[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_center(scroll_bar->children[i]);
        lv_obj_add_flag(scroll_bar->children[i], LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_clear_flag(scroll_bar->children[i], LV_OBJ_FLAG_SCROLLABLE);

        if(scroll_bar->item_childs_click_cb) {
            lv_obj_add_event_cb(scroll_bar->children[i], scroll_bar_child_click_cb, LV_EVENT_CLICKED, scroll_bar);
        }

        if(i == scroll_bar->selected) {
            lv_obj_set_size(scroll_bar->children[i], scroll_bar->item_size.width, scroll_bar->item_size.height);
            if(scroll_bar->item_childs_scale_cb) {
                scroll_bar->item_childs_scale_cb(scroll_bar->children[i], scroll_bar->scale_max);
            }
        } else {
            lv_obj_set_size(
                scroll_bar->children[i],
                (lv_coord_t)(scroll_bar->item_size.width * scroll_bar->scale_min),
                (lv_coord_t)(scroll_bar->item_size.height * scroll_bar->scale_min)
            );
            if(scroll_bar->item_childs_scale_cb) {
                scroll_bar->item_childs_scale_cb(scroll_bar->children[i], scroll_bar->scale_min);
            }
        }
    }

    if(scroll_bar->infinity_loop) {
        if(scroll_bar->child_nums % 2 == 0) {
            for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
                if(src_idx >= scroll_bar->child_nums) {
                    src_idx = 0;
                }
                if(i < (scroll_bar->child_nums / 2)) {
                    lv_obj_move_to_index(scroll_bar->children[src_idx], i + (scroll_bar->child_nums / 2));
                } else {
                    lv_obj_move_to_index(scroll_bar->children[src_idx], i - (scroll_bar->child_nums / 2));
                }
                src_idx++;
            }
        } else {
            for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
                if(src_idx >= scroll_bar->child_nums) {
                    src_idx = 0;
                }
                if(i <= (scroll_bar->child_nums / 2)) {
                    lv_obj_move_to_index(scroll_bar->children[src_idx], i + (scroll_bar->child_nums / 2));
                } else {
                    lv_obj_move_to_index(scroll_bar->children[src_idx], i - ((scroll_bar->child_nums + 1) / 2));
                }
                src_idx++;
            }
        }
    }

    for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
        lv_obj_t *child = lv_obj_get_child(scroll_bar->content, i);
        if(scroll_bar->direction == 0) {
            lv_obj_set_x(child, scroll_bar->spacing * i);
        } else {
            lv_obj_set_y(child, scroll_bar->spacing * i);
        }
    }

    lv_obj_scroll_to_view(scroll_bar->children[scroll_bar->selected], LV_ANIM_OFF);
    scroll_bar->last_scroll_pos = scroll_bar->direction == 0 ? lv_obj_get_scroll_x(scroll_bar->content) :
                                                             lv_obj_get_scroll_y(scroll_bar->content);
    lv_obj_add_flag(scroll_bar->children[scroll_bar->selected], LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(scroll_bar->content, scroll_bar_scroll_cb, LV_EVENT_SCROLL, scroll_bar);
    lv_obj_add_event_cb(scroll_bar->content, scroll_bar_scroll_end_cb, LV_EVENT_SCROLL_END, scroll_bar);
    lv_obj_add_event_cb(scroll_bar->content, scroll_bar_destroy_cb, LV_EVENT_DELETE, scroll_bar);
    return scroll_bar->content;
}

void scroll_bar_refresh(struct scroll_bar_s *scroll_bar)
{
    if((scroll_bar == NULL) || (scroll_bar->content == NULL) || (scroll_bar->children == NULL)) {
        return;
    }

    if(scroll_bar->selected >= scroll_bar->child_nums) {
        scroll_bar->selected = scroll_bar->child_nums - 1;
    }

    for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
        lv_obj_t *child = scroll_bar->children[i];
        if(child == NULL) {
            continue;
        }

        lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
        if(i == scroll_bar->selected) {
            lv_obj_set_size(child, scroll_bar->item_size.width, scroll_bar->item_size.height);
            if(scroll_bar->item_childs_scale_cb) {
                scroll_bar->item_childs_scale_cb(child, scroll_bar->scale_max);
            }
        } else {
            lv_obj_set_size(
                child,
                (lv_coord_t)(scroll_bar->item_size.width * scroll_bar->scale_min),
                (lv_coord_t)(scroll_bar->item_size.height * scroll_bar->scale_min)
            );
            if(scroll_bar->item_childs_scale_cb) {
                scroll_bar->item_childs_scale_cb(child, scroll_bar->scale_min);
            }
        }
    }

    for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
        lv_obj_t *child = lv_obj_get_child(scroll_bar->content, i);
        if(scroll_bar->direction == 0) {
            lv_obj_set_x(child, scroll_bar->spacing * i);
        } else {
            lv_obj_set_y(child, scroll_bar->spacing * i);
        }
    }

    lv_obj_scroll_to_view(scroll_bar->children[scroll_bar->selected], LV_ANIM_OFF);
    scroll_bar->last_scroll_pos = scroll_bar->direction == 0 ? lv_obj_get_scroll_x(scroll_bar->content) :
                                                             lv_obj_get_scroll_y(scroll_bar->content);
    lv_obj_add_flag(scroll_bar->children[scroll_bar->selected], LV_OBJ_FLAG_CLICKABLE);
}

static void scroll_bar_scroll_cb(lv_event_t *e)
{
    float item_scale = 1.0f;
    lv_obj_t *list = lv_event_get_target(e);
    struct scroll_bar_s *scroll_bar = lv_event_get_user_data(e);
    lv_area_t child_a;
    lv_area_t cont_a;
    int32_t child_x_center = 0;
    int32_t child_y_center = 0;
    int32_t diff_x = 0;
    int32_t diff_y = 0;
    int32_t cont_x_center = 0;
    int32_t cont_y_center = 0;
    int count = 0;
    lv_coord_t scrl_pos = 0;

    if((scroll_bar == NULL) || (scroll_bar->content == NULL) || (scroll_bar->children == NULL)) {
        return;
    }

    count = lv_obj_get_child_cnt(scroll_bar->content);
    lv_obj_get_coords(scroll_bar->content, &cont_a);
    cont_x_center = cont_a.x1 + lv_area_get_width(&cont_a) / 2;
    cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;
    scrl_pos = scroll_bar->direction == 0 ? lv_obj_get_scroll_x(scroll_bar->content) : lv_obj_get_scroll_y(scroll_bar->content);

    if(scroll_bar->timer && !MY_LV_TIMER_GET_PAUSED(scroll_bar->timer)) {
        lv_timer_pause(scroll_bar->timer);
    }

    if(scroll_bar->infinity_loop) {
        if(scrl_pos > (scroll_bar->spacing * scroll_bar->child_nums / 2 + scroll_bar->spacing)) {
            for(int i = 0; i < count - 1; i++) {
                lv_obj_swap(lv_obj_get_child(list, i), lv_obj_get_child(list, i + 1));
            }
            for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
                lv_obj_t *child = lv_obj_get_child(scroll_bar->content, i);
                if(scroll_bar->direction == 0) {
                    lv_obj_set_x(child, scroll_bar->spacing * i);
                } else {
                    lv_obj_set_y(child, scroll_bar->spacing * i);
                }
            }
            if(scroll_bar->direction == 0) {
                lv_obj_scroll_to_x(list, scrl_pos - scroll_bar->spacing, LV_ANIM_OFF);
            } else {
                lv_obj_scroll_to_y(list, scrl_pos - scroll_bar->spacing, LV_ANIM_OFF);
            }
        } else if(scrl_pos < (scroll_bar->spacing * scroll_bar->child_nums / 2 - scroll_bar->spacing)) {
            for(int i = count - 2; i >= 0; i--) {
                lv_obj_swap(lv_obj_get_child(list, i), lv_obj_get_child(list, i + 1));
            }
            for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
                lv_obj_t *child = lv_obj_get_child(scroll_bar->content, i);
                if(scroll_bar->direction == 0) {
                    lv_obj_set_x(child, scroll_bar->spacing * i);
                } else {
                    lv_obj_set_y(child, scroll_bar->spacing * i);
                }
            }
            if(scroll_bar->direction == 0) {
                lv_obj_scroll_to_x(list, scrl_pos + scroll_bar->spacing, LV_ANIM_OFF);
            } else {
                lv_obj_scroll_to_y(list, scrl_pos + scroll_bar->spacing, LV_ANIM_OFF);
            }
        }
    }

    for(uint16_t i = 0; i < scroll_bar->child_nums; i++) {
        lv_obj_t *child = lv_obj_get_child(scroll_bar->content, i);
        if(child == NULL) {
            continue;
        }
        lv_obj_get_coords(child, &child_a);
        if(scroll_bar->direction == 0) {
            child_x_center = child_a.x1 + lv_area_get_width(&child_a) / 2;
            diff_x = LV_ABS(child_x_center - cont_x_center);
            if(diff_x >= scroll_bar->spacing) {
                item_scale = scroll_bar->scale_min;
            } else {
                item_scale = scroll_bar->scale_min +
                             (scroll_bar->scale_max - scroll_bar->scale_min) * (float)(scroll_bar->spacing - diff_x) /
                                 scroll_bar->spacing;
            }
            if(diff_x < (scroll_bar->spacing / 2)) {
                for(uint16_t j = 0; j < scroll_bar->child_nums; j++) {
                    if(scroll_bar->children[j] == child) {
                        scroll_bar->selected = j;
                        break;
                    }
                }
                lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);
                if(scroll_bar->optimized) {
                    lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
                }
            } else {
                lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
                if(scroll_bar->optimized) {
                    if(diff_x < (lv_obj_get_width(scroll_bar->content) / 2 + scroll_bar->spacing / 2)) {
                        lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }
        } else {
            child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;
            diff_y = LV_ABS(child_y_center - cont_y_center);
            if(diff_y >= scroll_bar->spacing) {
                item_scale = scroll_bar->scale_min;
            } else {
                item_scale = scroll_bar->scale_min +
                             (scroll_bar->scale_max - scroll_bar->scale_min) * (float)(scroll_bar->spacing - diff_y) /
                                 scroll_bar->spacing;
            }
            if(diff_y < (scroll_bar->spacing / 2)) {
                for(uint16_t j = 0; j < scroll_bar->child_nums; j++) {
                    if(scroll_bar->children[j] == child) {
                        scroll_bar->selected = j;
                        break;
                    }
                }
                lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);
                if(scroll_bar->optimized) {
                    lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
                }
            } else {
                lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
                if(scroll_bar->optimized) {
                    if(diff_y < lv_obj_get_height(scroll_bar->content)) {
                        lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }
        }

        lv_coord_t target_w = (lv_coord_t)(scroll_bar->item_size.width * item_scale);
        lv_coord_t target_h = (lv_coord_t)(scroll_bar->item_size.height * item_scale);
        if((lv_obj_get_width(child) != target_w) || (lv_obj_get_height(child) != target_h)) {
            lv_obj_set_size(child, target_w, target_h);
            if(scroll_bar->item_childs_scale_cb) {
                scroll_bar->item_childs_scale_cb(child, item_scale);
            }
        }
    }

    if(!scroll_bar->infinity_loop && scroll_bar->edge_indicator) {
        lv_coord_t scroll_delta = scrl_pos - scroll_bar->last_scroll_pos;
        if(scroll_bar->direction == 0) {
            if((scroll_bar->selected == 0) && (scroll_delta < 0) && (lv_obj_get_scroll_left(scroll_bar->content) <= 0)) {
                scroll_bar_show_edge_indicator(scroll_bar, EDGE_LEFT);
            } else if((scroll_bar->selected == (scroll_bar->child_nums - 1)) && (scroll_delta > 0) &&
                      (lv_obj_get_scroll_right(scroll_bar->content) <= 0)) {
                scroll_bar_show_edge_indicator(scroll_bar, EDGE_RIGHT);
            }
        } else {
            if((scroll_bar->selected == 0) && (scroll_delta < 0) && (lv_obj_get_scroll_top(scroll_bar->content) <= 0)) {
                scroll_bar_show_edge_indicator(scroll_bar, EDGE_TOP);
            } else if((scroll_bar->selected == (scroll_bar->child_nums - 1)) && (scroll_delta > 0) &&
                      (lv_obj_get_scroll_bottom(scroll_bar->content) <= 0)) {
                scroll_bar_show_edge_indicator(scroll_bar, EDGE_BOTTOM);
            }
        }
    }
    scroll_bar->last_scroll_pos = scrl_pos;
}

static void scroll_bar_destroy_cb(lv_event_t *e)
{
    struct scroll_bar_s *scroll_bar = (struct scroll_bar_s *)lv_event_get_user_data(e);
    if(scroll_bar == NULL) {
        return;
    }

    if(scroll_bar->children) {
        free(scroll_bar->children);
        scroll_bar->children = NULL;
    }
    if(scroll_bar->timer) {
        lv_timer_del(scroll_bar->timer);
        scroll_bar->timer = NULL;
    }
    if(scroll_bar->edge_timer) {
        lv_timer_del(scroll_bar->edge_timer);
        scroll_bar->edge_timer = NULL;
    }
    if(scroll_bar->edge_bar) {
        lv_obj_del(scroll_bar->edge_bar);
        scroll_bar->edge_bar = NULL;
    }
}
