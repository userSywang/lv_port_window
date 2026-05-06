#pragma once

#include <string>
#include <vector>

#include "lvgl.h"
#include "esp_brookesia.hpp"

extern "C" {
#include "scroll_bar.h"
}

class ScrollTestApp : public ESP_Brookesia_PhoneApp {
public:
    ScrollTestApp();
    ~ScrollTestApp() override = default;

    bool run(void) override;
    bool back(void) override;
    bool close(void) override;
    bool init(void) override;

private:
    enum TranslationMode : uint8_t {
        ORIGINAL = 0,
        TRANSLATED = 1,
        ORIGINAL_AND_TRANSLATED = 2,
    };

    struct TranslationItemState {
        lv_obj_t *item = nullptr;
        uint32_t create_time = 0;
        bool is_latest = false;
        int index = 0;
    };

    struct SttItemState {
        lv_obj_t *item = nullptr;
        lv_obj_t *text_label = nullptr;
        uint32_t create_time = 0;
        bool is_latest = false;
        bool is_final = false;
        int index = 0;
    };

    struct FollowLineState {
        lv_obj_t *item = nullptr;
        lv_obj_t *text_label = nullptr;
    };

    static constexpr uint16_t kItemCount = 7;

    static void createScrollItem(lv_obj_t *item, uint16_t index);
    static void scaleScrollItem(lv_obj_t *item, float scale);
    static void onScrollItemClicked(lv_event_t *e);
    static void onScrollContentChanged(lv_event_t *e);
    static void onPageBackClicked(lv_event_t *e);
    static void onClockTimer(lv_timer_t *timer);
    static void onTranslateCleanupTimer(lv_timer_t *timer);
    static void onTranslateAddOriginal(lv_event_t *e);
    static void onTranslateAddTranslated(lv_event_t *e);
    static void onTranslateAddBoth(lv_event_t *e);
    static void onSttCleanupTimer(lv_timer_t *timer);
    static void onSttStreamTimer(lv_timer_t *timer);
    static void onSttSimulateClicked(lv_event_t *e);
    static void onFollowTimer(lv_timer_t *timer);

    void createHudOverlay(lv_coord_t width, lv_coord_t height);
    void createHomeView(lv_coord_t width, lv_coord_t height);
    void createPageView(lv_coord_t width, lv_coord_t height);
    lv_obj_t *createTranslationItem(lv_obj_t *parent, int index, const char *original_text, const char *translated_text, uint8_t mode);
    lv_obj_t *createSttItem(lv_obj_t *parent, int index, lv_obj_t **text_label);
    void ensureTranslateItems(void);
    void addTranslationItem(const char *original_text, const char *translated_text, TranslationMode mode);
    void setLatestTranslationItem(int index);
    void cleanupOldTranslationItems(void);
    void resetTranslationStream(void);
    void setTranslateControlsVisible(bool visible);
    void addSttItem(void);
    void setLatestSttItem(int index);
    void updateSttStream(void);
    void cleanupOldSttItems(void);
    void resetSttStream(void);
    void setSttControlsVisible(bool visible);
    void createFollowLines(void);
    void updateFollowHighlight(void);
    void resetFollowPrompt(void);
    void setFollowControlsVisible(bool visible);
    void createSettingsRows(void);
    void setSettingsControlsVisible(bool visible);
    void showHome(void);
    void showPage(uint16_t index);
    void updateClockDisplay(void);
    void updateTitleDisplay(void);

    lv_obj_t *_root = nullptr;
    lv_obj_t *_hud_overlay = nullptr;
    lv_obj_t *_home_view = nullptr;
    lv_obj_t *_page_view = nullptr;
    lv_obj_t *_page_badge = nullptr;
    lv_obj_t *_page_badge_label = nullptr;
    lv_obj_t *_page_title = nullptr;
    lv_obj_t *_page_hint = nullptr;
    lv_obj_t *_translate_scroll = nullptr;
    lv_obj_t *_translate_test_bar = nullptr;
    lv_obj_t *_translate_btn_orig = nullptr;
    lv_obj_t *_translate_btn_trans = nullptr;
    lv_obj_t *_translate_btn_both = nullptr;
    lv_obj_t *_stt_scroll = nullptr;
    lv_obj_t *_stt_test_bar = nullptr;
    lv_obj_t *_stt_simulate_btn = nullptr;
    lv_obj_t *_follow_scroll = nullptr;
    lv_obj_t *_settings_scroll = nullptr;
    lv_obj_t *_clock_time = nullptr;
    lv_obj_t *_clock_date = nullptr;
    lv_obj_t *_clock_weekday = nullptr;
    lv_obj_t *_title_labels[kItemCount] = {};
    int _active_page = -1;
    lv_timer_t *_clock_timer = nullptr;
    lv_timer_t *_translate_cleanup_timer = nullptr;
    lv_timer_t *_stt_cleanup_timer = nullptr;
    lv_timer_t *_stt_stream_timer = nullptr;
    lv_timer_t *_follow_timer = nullptr;
    scroll_bar_s _scroll_bar = {};
    std::vector<TranslationItemState> _translation_items;
    std::vector<SttItemState> _stt_items;
    std::vector<FollowLineState> _follow_lines;
    int _translate_current_index = 0;
    int _translate_latest_index = -1;
    uint32_t _translate_last_cleanup_time = 0;
    uint32_t _translate_seed_cursor = 0;
    int _stt_current_index = 0;
    int _stt_latest_index = -1;
    uint32_t _stt_last_cleanup_time = 0;
    uint32_t _stt_seed_cursor = 0;
    uint8_t _stt_stream_stage = 0;
    uint16_t _stt_stream_char = 0;
    uint8_t _stt_stream_hold_ticks = 0;
    std::string _stt_stream_text;
    uint16_t _follow_current_index = 0;
    uint8_t _follow_hold_ticks = 0;
};
