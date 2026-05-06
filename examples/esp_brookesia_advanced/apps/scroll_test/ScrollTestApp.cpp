#include "ScrollTestApp.hpp"

#include <cstdio>
#include <cstring>
#include <ctime>

#if defined(lv_timer_get_user_data)
#define MY_LV_TIMER_GET_USER_DATA(t) lv_timer_get_user_data(t)
#else
#define MY_LV_TIMER_GET_USER_DATA(t) ((t)->user_data)
#endif

namespace {
constexpr lv_color_t kBgColor = LV_COLOR_MAKE(0x00, 0x00, 0x00);
constexpr lv_color_t kGreen = LV_COLOR_MAKE(0x41, 0xff, 0x79);
constexpr lv_color_t kGreenSoft = LV_COLOR_MAKE(0x20, 0xc9, 0x58);
constexpr lv_color_t kGreenDim = LV_COLOR_MAKE(0x1a, 0x74, 0x33);
constexpr lv_color_t kTextWhite = LV_COLOR_MAKE(0xe8, 0xff, 0xee);
constexpr lv_color_t kTextDim = LV_COLOR_MAKE(0x78, 0x9d, 0x81);
const char *kWeekdays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const char *kItemTitles[] = {"Translate", "Prompt", "Live", "Service", "Guide", "FAQ", "Settings"};

struct TranslationSeed {
    const char *original;
    const char *translated;
    uint8_t mode;
};

const TranslationSeed kTranslateSeeds[] = {
    {"Source: Hello world", "English: Hello, world.", 2},
    {"Source: How are you today?", "English: How are you today?", 2},
    {"Source: Please open the camera", "English: Please open the camera.", 1},
    {"Source: Start recording now", "English: Start recording now.", 2},
    {"Source: See you tomorrow", "English: See you tomorrow.", 2},
};

struct SttSeed {
    const char *steps[4];
    uint8_t step_count;
};

const SttSeed kSttSeeds[] = {
    {{"Please open the cap", "Please open the camera", "Please open the camera and start recording"}, 3},
    {{"Set a timer four", "Set a timer for", "Set a timer for fifteen minutes"}, 3},
    {{"Call mister lee", "Call Mr Lee", "Call Mr Lee after the meeting"}, 3},
    {{"Turn on the life", "Turn on the light", "Turn on the living room light"}, 3},
};

const char *kFollowScript[] = {
    "Good morning everyone, thank you for joining this product update.",
    "Today I will walk through the design goals, the current progress, and the next milestones.",
    "The first goal is to keep the interface calm, readable, and useful during repeated daily work.",
    "The second goal is to reduce latency, so the speaker can trust the prompt while moving naturally.",
    "At this point, the recognition engine should mark the active sentence without hiding nearby context.",
    "The next sentence should already be prepared with a softer highlight before the speaker reaches it.",
    "When the speaker moves faster, the prompt can advance and keep the upcoming line visible.",
    "When the speaker pauses, the current line remains highlighted and the display stops drifting.",
    "This test page simulates that behavior with timed progress through a prepared script.",
    "The final implementation can replace the timer with real speech recognition progress callbacks.",
};

struct SettingsSeed {
    const char *group;
    const char *name;
    const char *detail;
    const char *value;
    bool enabled;
};

const SettingsSeed kSettingsSeeds[] = {
    {"CONNECT", "Wi-Fi", "Office network", "ON", true},
    {"CONNECT", "Bluetooth", "Phone paired", "ON", true},
    {"DISPLAY", "Brightness", "Adaptive lens display", "72%", true},
    {"DISPLAY", "Text Size", "Captions and prompts", "MID", true},
    {"AUDIO", "Mic Array", "Beamforming input", "ON", true},
    {"AUDIO", "Speaker", "Open-ear output", "45%", true},
    {"AI", "Wake Word", "Hands-free assistant", "ON", true},
    {"AI", "Live Translate", "Conversation overlay", "AUTO", true},
    {"AI", "Smart Follow", "Speech progress prompt", "ON", true},
    {"PRIVACY", "Camera Guard", "Recording indicator", "LOCK", true},
    {"PRIVACY", "Cloud Sync", "Upload transcripts", "OFF", false},
    {"SYSTEM", "Battery Saver", "Limit background AI", "OFF", false},
    {"SYSTEM", "Storage", "Local cache usage", "61%", true},
    {"SYSTEM", "Firmware", "Glasses runtime", "1.2.4", true},
};

static size_t common_prefix_len(const std::string &left, const char *right)
{
    if(right == nullptr) {
        return 0;
    }

    size_t index = 0;
    while((index < left.size()) && (right[index] != '\0') && (left[index] == right[index])) {
        index++;
    }
    return index;
}
}

static bool fill_local_time(std::tm *time_info)
{
    std::time_t now = std::time(nullptr);
    if(time_info == nullptr) {
        return false;
    }

#if defined(_WIN32)
    return localtime_s(time_info, &now) == 0;
#else
    return localtime_r(&now, time_info) != nullptr;
#endif
}

ScrollTestApp::ScrollTestApp() :
    ESP_Brookesia_PhoneApp("Scroll Test", nullptr, true, false, false)
{
}

bool ScrollTestApp::init(void)
{
    return true;
}

bool ScrollTestApp::run(void)
{
    const lv_area_t area = getVisualArea();
    const lv_coord_t width = area.x2 - area.x1 + 1;
    const lv_coord_t height = area.y2 - area.y1 + 1;

    _active_page = -1;

    _root = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(_root);
    lv_obj_set_size(_root, width, height);
    lv_obj_set_pos(_root, area.x1, area.y1);
    lv_obj_clear_flag(_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_root, kBgColor, 0);
    lv_obj_set_style_bg_opa(_root, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(_root, 0, 0);

    createHudOverlay(width, height);
    createHomeView(width, height);
    createPageView(width, height);
    showHome();
    updateClockDisplay();

    if(_clock_timer) {
        lv_timer_del(_clock_timer);
        _clock_timer = nullptr;
    }
    _clock_timer = lv_timer_create(onClockTimer, 1000, this);

    if(_translate_cleanup_timer) {
        lv_timer_del(_translate_cleanup_timer);
        _translate_cleanup_timer = nullptr;
    }
    _translate_cleanup_timer = lv_timer_create(onTranslateCleanupTimer, 1000, this);
    if(_translate_cleanup_timer) {
        lv_timer_pause(_translate_cleanup_timer);
    }

    if(_stt_cleanup_timer) {
        lv_timer_del(_stt_cleanup_timer);
        _stt_cleanup_timer = nullptr;
    }
    _stt_cleanup_timer = lv_timer_create(onSttCleanupTimer, 1300, this);
    if(_stt_cleanup_timer) {
        lv_timer_pause(_stt_cleanup_timer);
    }

    if(_stt_stream_timer) {
        lv_timer_del(_stt_stream_timer);
        _stt_stream_timer = nullptr;
    }
    _stt_stream_timer = lv_timer_create(onSttStreamTimer, 90, this);
    if(_stt_stream_timer) {
        lv_timer_pause(_stt_stream_timer);
    }

    if(_follow_timer) {
        lv_timer_del(_follow_timer);
        _follow_timer = nullptr;
    }
    _follow_timer = lv_timer_create(onFollowTimer, 950, this);
    if(_follow_timer) {
        lv_timer_pause(_follow_timer);
    }

    return true;
}

bool ScrollTestApp::back(void)
{
    if(_active_page >= 0) {
        showHome();
        return true;
    }

    return notifyCoreClosed();
}

bool ScrollTestApp::close(void)
{
    if(_clock_timer) {
        lv_timer_del(_clock_timer);
        _clock_timer = nullptr;
    }
    if(_translate_cleanup_timer) {
        lv_timer_del(_translate_cleanup_timer);
        _translate_cleanup_timer = nullptr;
    }
    if(_stt_cleanup_timer) {
        lv_timer_del(_stt_cleanup_timer);
        _stt_cleanup_timer = nullptr;
    }
    if(_stt_stream_timer) {
        lv_timer_del(_stt_stream_timer);
        _stt_stream_timer = nullptr;
    }
    if(_follow_timer) {
        lv_timer_del(_follow_timer);
        _follow_timer = nullptr;
    }
    resetTranslationStream();
    resetSttStream();
    resetFollowPrompt();
    _root = nullptr;
    _hud_overlay = nullptr;
    _home_view = nullptr;
    _page_view = nullptr;
    _page_badge = nullptr;
    _page_badge_label = nullptr;
    _page_title = nullptr;
    _page_hint = nullptr;
    _translate_scroll = nullptr;
    _translate_test_bar = nullptr;
    _translate_btn_orig = nullptr;
    _translate_btn_trans = nullptr;
    _translate_btn_both = nullptr;
    _stt_scroll = nullptr;
    _stt_test_bar = nullptr;
    _stt_simulate_btn = nullptr;
    _follow_scroll = nullptr;
    _settings_scroll = nullptr;
    _clock_time = nullptr;
    _clock_date = nullptr;
    _clock_weekday = nullptr;
    _active_page = -1;
    return true;
}

void ScrollTestApp::onClockTimer(lv_timer_t *timer)
{
    auto *app = static_cast<ScrollTestApp *>(timer ? MY_LV_TIMER_GET_USER_DATA(timer) : nullptr);
    if(app) {
        app->updateClockDisplay();
    }
}

void ScrollTestApp::onTranslateCleanupTimer(lv_timer_t *timer)
{
    auto *app = static_cast<ScrollTestApp *>(timer ? MY_LV_TIMER_GET_USER_DATA(timer) : nullptr);
    if(app) {
        app->cleanupOldTranslationItems();
    }
}

void ScrollTestApp::onSttCleanupTimer(lv_timer_t *timer)
{
    auto *app = static_cast<ScrollTestApp *>(timer ? MY_LV_TIMER_GET_USER_DATA(timer) : nullptr);
    if(app) {
        app->cleanupOldSttItems();
    }
}

void ScrollTestApp::onSttStreamTimer(lv_timer_t *timer)
{
    auto *app = static_cast<ScrollTestApp *>(timer ? MY_LV_TIMER_GET_USER_DATA(timer) : nullptr);
    if(app) {
        app->updateSttStream();
    }
}

void ScrollTestApp::onFollowTimer(lv_timer_t *timer)
{
    auto *app = static_cast<ScrollTestApp *>(timer ? MY_LV_TIMER_GET_USER_DATA(timer) : nullptr);
    if(app == nullptr || app->_follow_lines.empty()) {
        return;
    }

    app->_follow_hold_ticks++;
    if(app->_follow_hold_ticks < 2) {
        return;
    }

    app->_follow_hold_ticks = 0;
    app->_follow_current_index++;
    if(app->_follow_current_index >= app->_follow_lines.size()) {
        app->_follow_current_index = 0;
    }
    app->updateFollowHighlight();
}

void ScrollTestApp::onTranslateAddOriginal(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    if(app == nullptr) {
        return;
    }
    const auto &seed = kTranslateSeeds[app->_translate_seed_cursor % (sizeof(kTranslateSeeds) / sizeof(kTranslateSeeds[0]))];
    app->_translate_seed_cursor++;
    app->addTranslationItem(seed.original, seed.translated, ScrollTestApp::ORIGINAL);
}

void ScrollTestApp::onTranslateAddTranslated(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    if(app == nullptr) {
        return;
    }
    const auto &seed = kTranslateSeeds[app->_translate_seed_cursor % (sizeof(kTranslateSeeds) / sizeof(kTranslateSeeds[0]))];
    app->_translate_seed_cursor++;
    app->addTranslationItem(seed.original, seed.translated, ScrollTestApp::TRANSLATED);
}

void ScrollTestApp::onTranslateAddBoth(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    if(app == nullptr) {
        return;
    }
    const auto &seed = kTranslateSeeds[app->_translate_seed_cursor % (sizeof(kTranslateSeeds) / sizeof(kTranslateSeeds[0]))];
    app->_translate_seed_cursor++;
    app->addTranslationItem(seed.original, seed.translated, ScrollTestApp::ORIGINAL_AND_TRANSLATED);
}

void ScrollTestApp::onSttSimulateClicked(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    if(app) {
        app->addSttItem();
    }
}

void ScrollTestApp::updateClockDisplay(void)
{
    std::tm time_info = {};
    char time_buf[16];
    char date_buf[16];

    if((_clock_time == nullptr) || (_clock_date == nullptr) || (_clock_weekday == nullptr)) {
        return;
    }

    if(!fill_local_time(&time_info)) {
        return;
    }

    std::snprintf(time_buf, sizeof(time_buf), "%02d:%02d", time_info.tm_hour, time_info.tm_min);
    std::snprintf(date_buf, sizeof(date_buf), "%02d/%02d", time_info.tm_mon + 1, time_info.tm_mday);

    lv_label_set_text(_clock_time, time_buf);
    lv_label_set_text(_clock_date, date_buf);
    lv_label_set_text(_clock_weekday, kWeekdays[time_info.tm_wday % 7]);
}

void ScrollTestApp::createHudOverlay(lv_coord_t width, lv_coord_t height)
{
    LV_UNUSED(width);
    LV_UNUSED(height);

    _hud_overlay = lv_obj_create(_root);
    lv_obj_remove_style_all(_hud_overlay);
    lv_obj_set_size(_hud_overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(_hud_overlay, 0, 0);
    lv_obj_add_flag(_hud_overlay, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(_hud_overlay, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *status_row = lv_obj_create(_hud_overlay);
    lv_obj_remove_style_all(status_row);
    lv_obj_set_size(status_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(status_row, LV_ALIGN_TOP_LEFT, 18, 14);
    lv_obj_clear_flag(status_row, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_layout(status_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(status_row, 14, 0);

    lv_obj_t *wifi = lv_label_create(status_row);
    lv_label_set_text(wifi, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi, kGreen, 0);
    lv_obj_set_style_text_font(wifi, &lv_font_montserrat_16, 0);

    lv_obj_t *battery = lv_label_create(status_row);
    lv_label_set_text(battery, LV_SYMBOL_BATTERY_3);
    lv_obj_set_style_text_color(battery, kGreen, 0);
    lv_obj_set_style_text_font(battery, &lv_font_montserrat_16, 0);

    _clock_time = lv_label_create(_hud_overlay);
    lv_obj_set_style_text_color(_clock_time, kTextWhite, 0);
    lv_obj_set_style_text_font(_clock_time, &lv_font_montserrat_48, 0);
    lv_obj_align(_clock_time, LV_ALIGN_TOP_LEFT, 18, 44);

    _clock_date = lv_label_create(_hud_overlay);
    lv_obj_set_style_text_color(_clock_date, kTextDim, 0);
    lv_obj_set_style_text_font(_clock_date, &lv_font_montserrat_16, 0);
    lv_obj_align(_clock_date, LV_ALIGN_TOP_LEFT, 18, 104);

    _clock_weekday = lv_label_create(_hud_overlay);
    lv_obj_set_style_text_color(_clock_weekday, kGreenSoft, 0);
    lv_obj_set_style_text_font(_clock_weekday, &lv_font_montserrat_16, 0);
    lv_obj_align(_clock_weekday, LV_ALIGN_TOP_LEFT, 18, 128);
}

void ScrollTestApp::createHomeView(lv_coord_t width, lv_coord_t height)
{
    _home_view = lv_obj_create(_root);
    lv_obj_remove_style_all(_home_view);
    lv_obj_set_size(_home_view, width, height);
    lv_obj_set_pos(_home_view, 0, 0);
    lv_obj_clear_flag(_home_view, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(_home_view, LV_OBJ_FLAG_IGNORE_LAYOUT);

    lv_obj_t *host = lv_obj_create(_home_view);
    lv_obj_remove_style_all(host);
    lv_obj_set_size(host, width - 36, 240);
    lv_obj_align(host, LV_ALIGN_CENTER, 0, 96);
    lv_obj_clear_flag(host, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    for(uint16_t i = 0; i < kItemCount; ++i) {
        _title_labels[i] = lv_label_create(_home_view);
        lv_label_set_text(_title_labels[i], kItemTitles[i]);
        lv_obj_set_style_text_color(_title_labels[i], kTextWhite, 0);
        lv_obj_set_style_text_font(_title_labels[i], &lv_font_montserrat_16, 0);
        lv_obj_add_flag(_title_labels[i], LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);
    }

    _scroll_bar.child_nums = kItemCount;
    _scroll_bar.spacing = LV_MAX(width / 4, 132);
    _scroll_bar.selected = 0;
    _scroll_bar.direction = 0;
    _scroll_bar.onesnap = 1;
    _scroll_bar.optimized = 0;
    _scroll_bar.infinity_loop = 0;
    _scroll_bar.edge_indicator = 1;
    _scroll_bar.scale_min = 0.60f;
    _scroll_bar.scale_max = 1.0f;
    _scroll_bar.item_titles = kItemTitles;
    _scroll_bar.item_size.width = 164;
    _scroll_bar.item_size.height = 164;
    _scroll_bar.title_labels = _title_labels;
    _scroll_bar.edge_bar = nullptr;
    _scroll_bar.edge_timer = nullptr;
    _scroll_bar.item_childs_create_cb = createScrollItem;
    _scroll_bar.item_childs_scale_cb = scaleScrollItem;
    _scroll_bar.item_childs_click_cb = nullptr;

    scroll_bar_create(host, &_scroll_bar);
    lv_obj_add_event_cb(_scroll_bar.content, onScrollContentChanged, LV_EVENT_SCROLL, this);
    updateTitleDisplay();

    for(uint16_t i = 0; i < kItemCount; ++i) {
        lv_obj_add_event_cb(_scroll_bar.children[i], onScrollItemClicked, LV_EVENT_CLICKED, this);
    }
}

void ScrollTestApp::createPageView(lv_coord_t width, lv_coord_t height)
{
    _page_view = lv_obj_create(_root);
    lv_obj_remove_style_all(_page_view);
    lv_obj_set_size(_page_view, width, height);
    lv_obj_set_pos(_page_view, 0, 0);
    lv_obj_add_flag(_page_view, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(_page_view, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(_page_view, kBgColor, 0);
    lv_obj_set_style_bg_opa(_page_view, LV_OPA_COVER, 0);

    lv_obj_t *back_btn = lv_btn_create(_page_view);
    lv_obj_set_size(back_btn, 94, 42);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 18, 14);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_set_style_bg_color(back_btn, kBgColor, 0);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(back_btn, 2, 0);
    lv_obj_set_style_border_color(back_btn, kGreen, 0);
    lv_obj_add_event_cb(back_btn, onPageBackClicked, LV_EVENT_CLICKED, this);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "BACK");
    lv_obj_set_style_text_color(back_label, kGreen, 0);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_14, 0);
    lv_obj_center(back_label);

    _page_title = lv_label_create(_page_view);
    lv_obj_set_style_text_color(_page_title, kGreen, 0);
    lv_obj_set_style_text_font(_page_title, &lv_font_montserrat_24, 0);
    lv_obj_align(_page_title, LV_ALIGN_TOP_LEFT, 18, 80);

    _page_badge = lv_obj_create(_page_view);
    lv_obj_set_size(_page_badge, 188, 188);
    lv_obj_align(_page_badge, LV_ALIGN_LEFT_MID, 112, 30);
    lv_obj_clear_flag(_page_badge, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(_page_badge, 14, 0);
    lv_obj_set_style_bg_color(_page_badge, kBgColor, 0);
    lv_obj_set_style_bg_opa(_page_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_page_badge, 3, 0);
    lv_obj_set_style_border_color(_page_badge, kGreen, 0);

    _page_badge_label = lv_label_create(_page_badge);
    lv_obj_set_style_text_color(_page_badge_label, kTextWhite, 0);
    lv_obj_set_style_text_font(_page_badge_label, &lv_font_montserrat_48, 0);
    lv_obj_center(_page_badge_label);

    _page_hint = lv_label_create(_page_view);
    lv_obj_set_width(_page_hint, width - 360);
    lv_label_set_long_mode(_page_hint, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(_page_hint, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_color(_page_hint, kTextDim, 0);
    lv_obj_set_style_text_font(_page_hint, &lv_font_montserrat_14, 0);
    lv_obj_align(_page_hint, LV_ALIGN_LEFT_MID, 320, 18);

    _translate_scroll = lv_obj_create(_page_view);
    lv_obj_set_size(_translate_scroll, width - 36, height - 150);
    lv_obj_align(_translate_scroll, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_border_width(_translate_scroll, 0, 0);
    lv_obj_set_style_bg_color(_translate_scroll, kBgColor, 0);
    lv_obj_set_style_bg_opa(_translate_scroll, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(_translate_scroll, 10, 0);
    lv_obj_set_style_pad_row(_translate_scroll, 10, 0);
    lv_obj_set_scroll_dir(_translate_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_translate_scroll, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(_translate_scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_translate_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(_translate_scroll, LV_OBJ_FLAG_HIDDEN);

    _translate_test_bar = lv_obj_create(_page_view);
    lv_obj_set_size(_translate_test_bar, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(_translate_test_bar, LV_ALIGN_TOP_RIGHT, -18, 14);
    lv_obj_set_style_bg_color(_translate_test_bar, kBgColor, 0);
    lv_obj_set_style_bg_opa(_translate_test_bar, LV_OPA_60, 0);
    lv_obj_set_style_border_width(_translate_test_bar, 1, 0);
    lv_obj_set_style_border_color(_translate_test_bar, kGreenDim, 0);
    lv_obj_set_style_pad_all(_translate_test_bar, 6, 0);
    lv_obj_set_style_pad_column(_translate_test_bar, 6, 0);
    lv_obj_set_layout(_translate_test_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(_translate_test_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_translate_test_bar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(_translate_test_bar, LV_OBJ_FLAG_HIDDEN);

    _translate_btn_orig = lv_btn_create(_translate_test_bar);
    lv_obj_set_size(_translate_btn_orig, 62, 34);
    lv_obj_set_style_bg_color(_translate_btn_orig, kBgColor, 0);
    lv_obj_set_style_bg_opa(_translate_btn_orig, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_translate_btn_orig, 1, 0);
    lv_obj_set_style_border_color(_translate_btn_orig, kGreen, 0);
    lv_obj_add_event_cb(_translate_btn_orig, onTranslateAddOriginal, LV_EVENT_CLICKED, this);
    lv_obj_t *orig_label = lv_label_create(_translate_btn_orig);
    lv_label_set_text(orig_label, "ORIG");
    lv_obj_set_style_text_color(orig_label, kGreen, 0);
    lv_obj_set_style_text_font(orig_label, &lv_font_montserrat_12, 0);
    lv_obj_center(orig_label);

    _translate_btn_trans = lv_btn_create(_translate_test_bar);
    lv_obj_set_size(_translate_btn_trans, 62, 34);
    lv_obj_set_style_bg_color(_translate_btn_trans, kBgColor, 0);
    lv_obj_set_style_bg_opa(_translate_btn_trans, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_translate_btn_trans, 1, 0);
    lv_obj_set_style_border_color(_translate_btn_trans, kGreen, 0);
    lv_obj_add_event_cb(_translate_btn_trans, onTranslateAddTranslated, LV_EVENT_CLICKED, this);
    lv_obj_t *trans_label = lv_label_create(_translate_btn_trans);
    lv_label_set_text(trans_label, "TRANS");
    lv_obj_set_style_text_color(trans_label, kGreen, 0);
    lv_obj_set_style_text_font(trans_label, &lv_font_montserrat_12, 0);
    lv_obj_center(trans_label);

    _translate_btn_both = lv_btn_create(_translate_test_bar);
    lv_obj_set_size(_translate_btn_both, 62, 34);
    lv_obj_set_style_bg_color(_translate_btn_both, kBgColor, 0);
    lv_obj_set_style_bg_opa(_translate_btn_both, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_translate_btn_both, 1, 0);
    lv_obj_set_style_border_color(_translate_btn_both, kGreen, 0);
    lv_obj_add_event_cb(_translate_btn_both, onTranslateAddBoth, LV_EVENT_CLICKED, this);
    lv_obj_t *both_label = lv_label_create(_translate_btn_both);
    lv_label_set_text(both_label, "BOTH");
    lv_obj_set_style_text_color(both_label, kGreen, 0);
    lv_obj_set_style_text_font(both_label, &lv_font_montserrat_12, 0);
    lv_obj_center(both_label);

    _stt_scroll = lv_obj_create(_page_view);
    lv_obj_set_size(_stt_scroll, width - 36, height - 150);
    lv_obj_align(_stt_scroll, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_border_width(_stt_scroll, 0, 0);
    lv_obj_set_style_bg_color(_stt_scroll, kBgColor, 0);
    lv_obj_set_style_bg_opa(_stt_scroll, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(_stt_scroll, 10, 0);
    lv_obj_set_style_pad_row(_stt_scroll, 10, 0);
    lv_obj_set_scroll_dir(_stt_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_stt_scroll, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(_stt_scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_stt_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(_stt_scroll, LV_OBJ_FLAG_HIDDEN);

    _stt_test_bar = lv_obj_create(_page_view);
    lv_obj_set_size(_stt_test_bar, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(_stt_test_bar, LV_ALIGN_TOP_RIGHT, -18, 14);
    lv_obj_set_style_bg_color(_stt_test_bar, kBgColor, 0);
    lv_obj_set_style_bg_opa(_stt_test_bar, LV_OPA_60, 0);
    lv_obj_set_style_border_width(_stt_test_bar, 1, 0);
    lv_obj_set_style_border_color(_stt_test_bar, kGreenDim, 0);
    lv_obj_set_style_pad_all(_stt_test_bar, 6, 0);
    lv_obj_clear_flag(_stt_test_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_stt_test_bar, LV_OBJ_FLAG_HIDDEN);

    _stt_simulate_btn = lv_btn_create(_stt_test_bar);
    lv_obj_set_size(_stt_simulate_btn, 74, 34);
    lv_obj_set_style_bg_color(_stt_simulate_btn, kBgColor, 0);
    lv_obj_set_style_bg_opa(_stt_simulate_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_stt_simulate_btn, 1, 0);
    lv_obj_set_style_border_color(_stt_simulate_btn, kGreen, 0);
    lv_obj_add_event_cb(_stt_simulate_btn, onSttSimulateClicked, LV_EVENT_CLICKED, this);
    lv_obj_t *stt_label = lv_label_create(_stt_simulate_btn);
    lv_label_set_text(stt_label, "STT");
    lv_obj_set_style_text_color(stt_label, kGreen, 0);
    lv_obj_set_style_text_font(stt_label, &lv_font_montserrat_12, 0);
    lv_obj_center(stt_label);

    _follow_scroll = lv_obj_create(_page_view);
    lv_obj_set_size(_follow_scroll, width - 36, height - 150);
    lv_obj_align(_follow_scroll, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_border_width(_follow_scroll, 0, 0);
    lv_obj_set_style_bg_color(_follow_scroll, kBgColor, 0);
    lv_obj_set_style_bg_opa(_follow_scroll, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_top(_follow_scroll, height / 3, 0);
    lv_obj_set_style_pad_bottom(_follow_scroll, height / 3, 0);
    lv_obj_set_style_pad_left(_follow_scroll, 18, 0);
    lv_obj_set_style_pad_right(_follow_scroll, 18, 0);
    lv_obj_set_style_pad_row(_follow_scroll, 12, 0);
    lv_obj_set_scroll_dir(_follow_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_follow_scroll, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(_follow_scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_follow_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(_follow_scroll, LV_OBJ_FLAG_HIDDEN);

    _settings_scroll = lv_obj_create(_page_view);
    lv_obj_set_size(_settings_scroll, width - 36, height - 150);
    lv_obj_align(_settings_scroll, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_border_width(_settings_scroll, 0, 0);
    lv_obj_set_style_bg_color(_settings_scroll, kBgColor, 0);
    lv_obj_set_style_bg_opa(_settings_scroll, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(_settings_scroll, 10, 0);
    lv_obj_set_style_pad_row(_settings_scroll, 8, 0);
    lv_obj_set_scroll_dir(_settings_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_settings_scroll, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(_settings_scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_settings_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(_settings_scroll, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t *ScrollTestApp::createTranslationItem(
    lv_obj_t *parent, int index, const char *original_text, const char *translated_text, uint8_t mode)
{
    const lv_coord_t item_width = lv_obj_get_width(parent) - 20;
    const lv_coord_t text_x = 62;
    const lv_coord_t text_width = item_width - text_x - 12;

    lv_obj_t *item_cont = lv_obj_create(parent);
    lv_obj_set_size(item_cont, item_width, 132);
    lv_obj_set_style_border_width(item_cont, 0, 0);
    lv_obj_set_style_pad_all(item_cont, 0, 0);
    lv_obj_set_style_bg_color(item_cont, LV_COLOR_MAKE(0x10, 0x15, 0x2F), 0);
    lv_obj_set_style_bg_opa(item_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(item_cont, 8, 0);
    lv_obj_set_style_opa(item_cont, LV_OPA_50, 0);
    lv_obj_set_scrollbar_mode(item_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(item_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *num_label = lv_label_create(item_cont);
    lv_obj_set_size(num_label, 40, 40);
    lv_label_set_text_fmt(num_label, "%d", index);
    lv_obj_set_style_text_color(num_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(num_label, &lv_font_montserrat_24, 0);
    lv_obj_align(num_label, LV_ALIGN_LEFT_MID, 10, 0);

    if(mode == ORIGINAL_AND_TRANSLATED) {
        lv_obj_t *separator = lv_obj_create(item_cont);
        lv_obj_set_size(separator, text_width, 2);
        lv_obj_set_style_bg_color(separator, lv_color_hex(0x4A5568), 0);
        lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(separator, 0, 0);
        lv_obj_set_style_radius(separator, 0, 0);
        lv_obj_set_pos(separator, text_x, 65);
        lv_obj_clear_flag(separator, LV_OBJ_FLAG_SCROLLABLE);
    }

    if(mode != ORIGINAL) {
        lv_obj_t *trans_label = lv_label_create(item_cont);
        lv_obj_set_size(trans_label, text_width, mode == ORIGINAL_AND_TRANSLATED ? 42 : 70);
        lv_label_set_text(trans_label, translated_text);
        lv_obj_set_style_text_color(trans_label, lv_color_hex(0x41FF79), 0);
        lv_obj_set_style_text_opa(trans_label, LV_OPA_COVER, 0);
        lv_obj_set_style_text_font(trans_label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_align(trans_label, LV_TEXT_ALIGN_LEFT, 0);
        if(mode == ORIGINAL_AND_TRANSLATED) {
            lv_obj_set_pos(trans_label, text_x, 18);
        } else {
            lv_obj_set_pos(trans_label, text_x, 31);
        }
        lv_label_set_long_mode(trans_label, LV_LABEL_LONG_WRAP);
    }

    if(mode != TRANSLATED) {
        lv_obj_t *orig_label = lv_label_create(item_cont);
        lv_obj_set_size(orig_label, text_width, mode == ORIGINAL_AND_TRANSLATED ? 42 : 70);
        lv_label_set_text(orig_label, original_text);
        lv_obj_set_style_text_color(orig_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(orig_label, LV_OPA_COVER, 0);
        lv_obj_set_style_text_font(orig_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_align(orig_label, LV_TEXT_ALIGN_LEFT, 0);
        if(mode == ORIGINAL_AND_TRANSLATED) {
            lv_obj_set_pos(orig_label, text_x, 75);
        } else {
            lv_obj_set_pos(orig_label, text_x, 31);
        }
        lv_label_set_long_mode(orig_label, LV_LABEL_LONG_WRAP);
    }

    return item_cont;
}

lv_obj_t *ScrollTestApp::createSttItem(lv_obj_t *parent, int index, lv_obj_t **text_label)
{
    const lv_coord_t item_width = lv_obj_get_width(parent) - 20;
    const lv_coord_t text_x = 62;
    const lv_coord_t text_width = item_width - text_x - 12;

    lv_obj_t *item_cont = lv_obj_create(parent);
    lv_obj_set_size(item_cont, item_width, 112);
    lv_obj_set_style_border_width(item_cont, 0, 0);
    lv_obj_set_style_pad_all(item_cont, 0, 0);
    lv_obj_set_style_bg_color(item_cont, LV_COLOR_MAKE(0x10, 0x15, 0x2F), 0);
    lv_obj_set_style_bg_opa(item_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(item_cont, 8, 0);
    lv_obj_set_style_opa(item_cont, LV_OPA_50, 0);
    lv_obj_set_scrollbar_mode(item_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(item_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *num_label = lv_label_create(item_cont);
    lv_obj_set_size(num_label, 40, 40);
    lv_label_set_text_fmt(num_label, "%d", index);
    lv_obj_set_style_text_color(num_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(num_label, &lv_font_montserrat_24, 0);
    lv_obj_align(num_label, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t *status_label = lv_label_create(item_cont);
    lv_label_set_text(status_label, "VOICE");
    lv_obj_set_style_text_color(status_label, kGreenSoft, 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(status_label, text_x, 14);

    lv_obj_t *body_label = lv_label_create(item_cont);
    lv_obj_set_size(body_label, text_width, 66);
    lv_label_set_text(body_label, "");
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(body_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(body_label, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(body_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(body_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(body_label, text_x, 38);

    if(text_label) {
        *text_label = body_label;
    }

    return item_cont;
}

void ScrollTestApp::ensureTranslateItems(void)
{
    if(_translate_scroll == nullptr) {
        return;
    }
    if(!_translation_items.empty()) {
        return;
    }

    const auto &seed = kTranslateSeeds[0];
    addTranslationItem(seed.original, seed.translated, ORIGINAL_AND_TRANSLATED);
}

void ScrollTestApp::addTranslationItem(const char *original_text, const char *translated_text, TranslationMode mode)
{
    if(_translate_scroll == nullptr) {
        return;
    }

    int new_index = ++_translate_current_index;
    lv_obj_t *item = createTranslationItem(_translate_scroll, new_index, original_text, translated_text, (uint8_t)mode);
    if(item == nullptr) {
        return;
    }

    TranslationItemState state;
    state.item = item;
    state.create_time = lv_tick_get();
    state.is_latest = true;
    state.index = new_index;
    _translation_items.push_back(state);

    setLatestTranslationItem(new_index);
    lv_obj_move_foreground(item);
    lv_obj_scroll_to_view(item, LV_ANIM_ON);
}

void ScrollTestApp::setLatestTranslationItem(int index)
{
    if(_translate_latest_index != -1) {
        for(auto &entry : _translation_items) {
            if(entry.index == _translate_latest_index) {
                entry.is_latest = false;
                entry.create_time = lv_tick_get();
                if(entry.item) {
                    lv_obj_set_style_opa(entry.item, LV_OPA_50, 0);
                }
                break;
            }
        }
    }

    for(auto &entry : _translation_items) {
        if(entry.index == index) {
            entry.is_latest = true;
            entry.create_time = lv_tick_get();
            if(entry.item) {
                lv_obj_set_style_opa(entry.item, LV_OPA_100, 0);
                lv_obj_clear_flag(entry.item, LV_OBJ_FLAG_HIDDEN);
            }
            break;
        }
    }

    _translate_latest_index = index;
}

void ScrollTestApp::cleanupOldTranslationItems(void)
{
    if(_translate_scroll == nullptr) {
        return;
    }

    uint32_t now = lv_tick_get();
    const uint32_t kCleanupThresholdMs = 3000;
    if((now - _translate_last_cleanup_time) < 1000) {
        return;
    }
    _translate_last_cleanup_time = now;

    for(auto it = _translation_items.begin(); it != _translation_items.end();) {
        bool should_delete = (!it->is_latest) && ((now - it->create_time) > kCleanupThresholdMs);
        if(should_delete) {
            if(it->item) {
                lv_obj_del(it->item);
                it->item = nullptr;
            }
            it = _translation_items.erase(it);
        } else {
            ++it;
        }
    }
}

void ScrollTestApp::resetTranslationStream(void)
{
    for(auto &entry : _translation_items) {
        if(entry.item) {
            lv_obj_del(entry.item);
            entry.item = nullptr;
        }
    }
    _translation_items.clear();
    _translate_current_index = 0;
    _translate_latest_index = -1;
    _translate_last_cleanup_time = 0;
    _translate_seed_cursor = 0;
}

void ScrollTestApp::addSttItem(void)
{
    if(_stt_scroll == nullptr) {
        return;
    }

    lv_obj_t *text_label = nullptr;
    int new_index = ++_stt_current_index;
    lv_obj_t *item = createSttItem(_stt_scroll, new_index, &text_label);
    if((item == nullptr) || (text_label == nullptr)) {
        return;
    }

    SttItemState state;
    state.item = item;
    state.text_label = text_label;
    state.create_time = lv_tick_get();
    state.is_latest = true;
    state.is_final = false;
    state.index = new_index;
    _stt_items.push_back(state);

    setLatestSttItem(new_index);
    _stt_stream_stage = 0;
    _stt_stream_char = 0;
    _stt_stream_hold_ticks = 0;
    _stt_stream_text.clear();
    _stt_seed_cursor++;
    lv_label_set_text(text_label, "");
    lv_obj_move_foreground(item);
    lv_obj_scroll_to_view(item, LV_ANIM_ON);

    if(_stt_stream_timer) {
        lv_timer_resume(_stt_stream_timer);
    }
}

void ScrollTestApp::setLatestSttItem(int index)
{
    if(_stt_latest_index != -1) {
        for(auto &entry : _stt_items) {
            if(entry.index == _stt_latest_index) {
                entry.is_latest = false;
                entry.is_final = true;
                entry.create_time = lv_tick_get();
                if(entry.item) {
                    lv_obj_set_style_opa(entry.item, LV_OPA_50, 0);
                }
                break;
            }
        }
    }

    for(auto &entry : _stt_items) {
        if(entry.index == index) {
            entry.is_latest = true;
            entry.is_final = false;
            entry.create_time = lv_tick_get();
            if(entry.item) {
                lv_obj_set_style_opa(entry.item, LV_OPA_100, 0);
                lv_obj_clear_flag(entry.item, LV_OBJ_FLAG_HIDDEN);
            }
            break;
        }
    }

    _stt_latest_index = index;
}

void ScrollTestApp::updateSttStream(void)
{
    if((_stt_latest_index < 0) || _stt_items.empty()) {
        if(_stt_stream_timer) {
            lv_timer_pause(_stt_stream_timer);
        }
        return;
    }

    SttItemState *latest = nullptr;
    for(auto &entry : _stt_items) {
        if(entry.index == _stt_latest_index) {
            latest = &entry;
            break;
        }
    }
    if((latest == nullptr) || (latest->text_label == nullptr)) {
        return;
    }

    const size_t seed_index = (size_t)((_stt_latest_index - 1) % (sizeof(kSttSeeds) / sizeof(kSttSeeds[0])));
    const SttSeed &seed = kSttSeeds[seed_index];
    if(_stt_stream_stage >= seed.step_count) {
        latest->is_final = true;
        latest->create_time = lv_tick_get();
        if(_stt_stream_timer) {
            lv_timer_pause(_stt_stream_timer);
        }
        return;
    }

    const char *target = seed.steps[_stt_stream_stage];
    const size_t target_len = std::strlen(target);
    if(_stt_stream_char < target_len) {
        _stt_stream_char++;
        _stt_stream_text.assign(target, _stt_stream_char);
        lv_label_set_text(latest->text_label, _stt_stream_text.c_str());
        lv_obj_scroll_to_view(latest->item, LV_ANIM_OFF);
        return;
    }

    _stt_stream_hold_ticks++;
    if(_stt_stream_hold_ticks < 4) {
        return;
    }

    _stt_stream_hold_ticks = 0;
    _stt_stream_stage++;
    if(_stt_stream_stage >= seed.step_count) {
        latest->is_final = true;
        latest->create_time = lv_tick_get();
        if(latest->item) {
            lv_obj_set_style_opa(latest->item, LV_OPA_100, 0);
        }
        if(_stt_stream_timer) {
            lv_timer_pause(_stt_stream_timer);
        }
        return;
    }

    target = seed.steps[_stt_stream_stage];
    _stt_stream_char = (uint16_t)common_prefix_len(_stt_stream_text, target);
    _stt_stream_text.assign(target, _stt_stream_char);
    lv_label_set_text(latest->text_label, _stt_stream_text.c_str());
}

void ScrollTestApp::cleanupOldSttItems(void)
{
    if(_stt_scroll == nullptr) {
        return;
    }

    uint32_t now = lv_tick_get();
    if((now - _stt_last_cleanup_time) < 1000) {
        return;
    }
    _stt_last_cleanup_time = now;

    for(auto it = _stt_items.begin(); it != _stt_items.end();) {
        const uint32_t cleanup_threshold_ms = 4500 + (uint32_t)((it->index % 3) * 1200);
        bool should_delete = (!it->is_latest) && it->is_final && ((now - it->create_time) > cleanup_threshold_ms);
        if(should_delete) {
            if(it->item) {
                lv_obj_del(it->item);
                it->item = nullptr;
                it->text_label = nullptr;
            }
            it = _stt_items.erase(it);
        } else {
            ++it;
        }
    }
}

void ScrollTestApp::resetSttStream(void)
{
    for(auto &entry : _stt_items) {
        if(entry.item) {
            lv_obj_del(entry.item);
            entry.item = nullptr;
            entry.text_label = nullptr;
        }
    }
    _stt_items.clear();
    _stt_current_index = 0;
    _stt_latest_index = -1;
    _stt_last_cleanup_time = 0;
    _stt_seed_cursor = 0;
    _stt_stream_stage = 0;
    _stt_stream_char = 0;
    _stt_stream_hold_ticks = 0;
    _stt_stream_text.clear();
    if(_stt_stream_timer) {
        lv_timer_pause(_stt_stream_timer);
    }
}

void ScrollTestApp::setTranslateControlsVisible(bool visible)
{
    if(_translate_scroll) {
        if(visible) {
            lv_obj_clear_flag(_translate_scroll, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_translate_scroll, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(_translate_test_bar) {
        if(visible) {
            lv_obj_clear_flag(_translate_test_bar, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_translate_test_bar, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(_translate_cleanup_timer) {
        if(visible) {
            lv_timer_resume(_translate_cleanup_timer);
        } else {
            lv_timer_pause(_translate_cleanup_timer);
        }
    }
}

void ScrollTestApp::setSttControlsVisible(bool visible)
{
    if(_stt_scroll) {
        if(visible) {
            lv_obj_clear_flag(_stt_scroll, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_stt_scroll, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(_stt_test_bar) {
        if(visible) {
            lv_obj_clear_flag(_stt_test_bar, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_stt_test_bar, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(_stt_cleanup_timer) {
        if(visible) {
            lv_timer_resume(_stt_cleanup_timer);
        } else {
            lv_timer_pause(_stt_cleanup_timer);
        }
    }
    if(_stt_stream_timer) {
        bool has_active_stream = false;
        for(const auto &entry : _stt_items) {
            if(entry.is_latest && !entry.is_final) {
                has_active_stream = true;
                break;
            }
        }
        if(visible && has_active_stream) {
            lv_timer_resume(_stt_stream_timer);
        } else {
            lv_timer_pause(_stt_stream_timer);
        }
    }
}

void ScrollTestApp::createFollowLines(void)
{
    if((_follow_scroll == nullptr) || !_follow_lines.empty()) {
        return;
    }

    const lv_coord_t line_width = lv_obj_get_width(_follow_scroll) - 36;
    for(size_t i = 0; i < (sizeof(kFollowScript) / sizeof(kFollowScript[0])); ++i) {
        lv_obj_t *item = lv_obj_create(_follow_scroll);
        lv_obj_set_size(item, line_width, LV_SIZE_CONTENT);
        lv_obj_set_style_border_width(item, 0, 0);
        lv_obj_set_style_bg_color(item, LV_COLOR_MAKE(0x07, 0x16, 0x0c), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_20, 0);
        lv_obj_set_style_radius(item, 8, 0);
        lv_obj_set_style_pad_all(item, 10, 0);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *label = lv_label_create(item);
        lv_obj_set_width(label, line_width - 20);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_label_set_text(label, kFollowScript[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(label);

        FollowLineState state;
        state.item = item;
        state.text_label = label;
        _follow_lines.push_back(state);
    }
}

void ScrollTestApp::updateFollowHighlight(void)
{
    if((_follow_scroll == nullptr) || _follow_lines.empty()) {
        return;
    }

    const uint16_t preview_index = (uint16_t)((_follow_current_index + 1) % _follow_lines.size());
    for(uint16_t i = 0; i < _follow_lines.size(); ++i) {
        FollowLineState &line = _follow_lines[i];
        if((line.item == nullptr) || (line.text_label == nullptr)) {
            continue;
        }

        if(i == _follow_current_index) {
            lv_obj_set_style_bg_opa(line.item, LV_OPA_50, 0);
            lv_obj_set_style_bg_color(line.item, LV_COLOR_MAKE(0x12, 0x38, 0x1d), 0);
            lv_obj_set_style_text_color(line.text_label, kGreen, 0);
            lv_obj_set_style_text_opa(line.text_label, LV_OPA_COVER, 0);
            lv_obj_set_style_text_font(line.text_label, &lv_font_montserrat_24, 0);
        } else if(i == preview_index) {
            lv_obj_set_style_bg_opa(line.item, LV_OPA_30, 0);
            lv_obj_set_style_bg_color(line.item, LV_COLOR_MAKE(0x0b, 0x25, 0x13), 0);
            lv_obj_set_style_text_color(line.text_label, kGreenSoft, 0);
            lv_obj_set_style_text_opa(line.text_label, LV_OPA_COVER, 0);
            lv_obj_set_style_text_font(line.text_label, &lv_font_montserrat_16, 0);
        } else {
            lv_obj_set_style_bg_opa(line.item, LV_OPA_10, 0);
            lv_obj_set_style_bg_color(line.item, LV_COLOR_MAKE(0x05, 0x10, 0x09), 0);
            lv_obj_set_style_text_color(line.text_label, kGreenDim, 0);
            lv_obj_set_style_text_opa(line.text_label, LV_OPA_70, 0);
            lv_obj_set_style_text_font(line.text_label, &lv_font_montserrat_16, 0);
        }
    }

    lv_obj_scroll_to_view(_follow_lines[_follow_current_index].item, LV_ANIM_ON);
}

void ScrollTestApp::resetFollowPrompt(void)
{
    for(auto &line : _follow_lines) {
        if(line.item) {
            lv_obj_del(line.item);
            line.item = nullptr;
            line.text_label = nullptr;
        }
    }
    _follow_lines.clear();
    _follow_current_index = 0;
    _follow_hold_ticks = 0;
    if(_follow_timer) {
        lv_timer_pause(_follow_timer);
    }
}

void ScrollTestApp::setFollowControlsVisible(bool visible)
{
    if(_follow_scroll) {
        if(visible) {
            lv_obj_clear_flag(_follow_scroll, LV_OBJ_FLAG_HIDDEN);
            createFollowLines();
            updateFollowHighlight();
        } else {
            lv_obj_add_flag(_follow_scroll, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(_follow_timer) {
        if(visible) {
            lv_timer_resume(_follow_timer);
        } else {
            lv_timer_pause(_follow_timer);
        }
    }
}

void ScrollTestApp::createSettingsRows(void)
{
    if(_settings_scroll == nullptr || lv_obj_get_child_cnt(_settings_scroll) > 0) {
        return;
    }

    const lv_coord_t row_width = lv_obj_get_width(_settings_scroll) - 20;
    const lv_coord_t value_width = 86;
    const lv_coord_t text_width = row_width - value_width - 34;
    const char *last_group = "";

    for(size_t i = 0; i < (sizeof(kSettingsSeeds) / sizeof(kSettingsSeeds[0])); ++i) {
        const SettingsSeed &setting = kSettingsSeeds[i];
        lv_color_t row_border_color = kGreenDim;
        lv_color_t row_bg_color = LV_COLOR_MAKE(0x08, 0x18, 0x0d);
        if(!setting.enabled) {
            row_border_color = LV_COLOR_MAKE(0x22, 0x36, 0x28);
            row_bg_color = LV_COLOR_MAKE(0x05, 0x0b, 0x07);
        }

        if(std::strcmp(last_group, setting.group) != 0) {
            lv_obj_t *group_label = lv_label_create(_settings_scroll);
            lv_obj_set_width(group_label, row_width);
            lv_label_set_text(group_label, setting.group);
            lv_obj_set_style_text_color(group_label, kGreenSoft, 0);
            lv_obj_set_style_text_font(group_label, &lv_font_montserrat_12, 0);
            lv_obj_set_style_pad_top(group_label, i == 0 ? 0 : 8, 0);
            last_group = setting.group;
        }

        lv_obj_t *row = lv_obj_create(_settings_scroll);
        lv_obj_set_size(row, row_width, 58);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, row_border_color, 0);
        lv_obj_set_style_bg_color(row, row_bg_color, 0);
        lv_obj_set_style_bg_opa(row, setting.enabled ? LV_OPA_70 : LV_OPA_50, 0);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *name_label = lv_label_create(row);
        lv_obj_set_size(name_label, text_width, 22);
        lv_label_set_text(name_label, setting.name);
        lv_label_set_long_mode(name_label, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(name_label, setting.enabled ? kTextWhite : kTextDim, 0);
        lv_obj_set_style_text_font(name_label, &lv_font_montserrat_16, 0);
        lv_obj_set_pos(name_label, 12, 8);

        lv_obj_t *detail_label = lv_label_create(row);
        lv_obj_set_size(detail_label, text_width, 18);
        lv_label_set_text(detail_label, setting.detail);
        lv_label_set_long_mode(detail_label, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(detail_label, kTextDim, 0);
        lv_obj_set_style_text_font(detail_label, &lv_font_montserrat_12, 0);
        lv_obj_set_pos(detail_label, 12, 34);

        lv_obj_t *value_box = lv_obj_create(row);
        lv_obj_set_size(value_box, value_width, 32);
        lv_obj_set_style_border_width(value_box, 1, 0);
        lv_obj_set_style_border_color(value_box, setting.enabled ? kGreen : kGreenDim, 0);
        lv_obj_set_style_bg_color(value_box, kBgColor, 0);
        lv_obj_set_style_bg_opa(value_box, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(value_box, 8, 0);
        lv_obj_clear_flag(value_box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(value_box, LV_ALIGN_RIGHT_MID, -10, 0);

        lv_obj_t *value_label = lv_label_create(value_box);
        lv_label_set_text(value_label, setting.value);
        lv_obj_set_style_text_color(value_label, setting.enabled ? kGreen : kTextDim, 0);
        lv_obj_set_style_text_font(value_label, &lv_font_montserrat_12, 0);
        lv_obj_center(value_label);
    }
}

void ScrollTestApp::setSettingsControlsVisible(bool visible)
{
    if(_settings_scroll) {
        if(visible) {
            createSettingsRows();
            lv_obj_clear_flag(_settings_scroll, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_y(_settings_scroll, 0, LV_ANIM_OFF);
        } else {
            lv_obj_add_flag(_settings_scroll, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void ScrollTestApp::showHome(void)
{
    _active_page = -1;
    setTranslateControlsVisible(false);
    setSttControlsVisible(false);
    setFollowControlsVisible(false);
    setSettingsControlsVisible(false);
    if(_home_view) {
        lv_obj_clear_flag(_home_view, LV_OBJ_FLAG_HIDDEN);
    }
    if(_page_view) {
        lv_obj_add_flag(_page_view, LV_OBJ_FLAG_HIDDEN);
    }
}

void ScrollTestApp::showPage(uint16_t index)
{
    char title[32];
    char badge_text[8];
    char hint[128];

    _active_page = static_cast<int>(index);
    std::snprintf(title, sizeof(title), "Function %u", index + 1);
    std::snprintf(badge_text, sizeof(badge_text), "%u", index + 1);
    std::snprintf(hint, sizeof(hint),
                  "Temporary test page for item %u.\nUse this screen to connect the real feature flow later.",
                  index + 1);

    if(index == 0) {
        lv_label_set_text(_page_title, "Function 1 - Translate");
        lv_obj_add_flag(_page_badge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_page_hint, LV_OBJ_FLAG_HIDDEN);
        setTranslateControlsVisible(true);
        setSttControlsVisible(false);
        setFollowControlsVisible(false);
        setSettingsControlsVisible(false);
        ensureTranslateItems();
    } else if(index == 1) {
        lv_label_set_text(_page_title, "Function 2 - Speech Text");
        lv_obj_add_flag(_page_badge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_page_hint, LV_OBJ_FLAG_HIDDEN);
        setTranslateControlsVisible(false);
        setSttControlsVisible(true);
        setFollowControlsVisible(false);
        setSettingsControlsVisible(false);
    } else if(index == 2) {
        lv_label_set_text(_page_title, "Function 3 - Smart Follow");
        lv_obj_add_flag(_page_badge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_page_hint, LV_OBJ_FLAG_HIDDEN);
        setTranslateControlsVisible(false);
        setSttControlsVisible(false);
        setFollowControlsVisible(true);
        setSettingsControlsVisible(false);
    } else if(index == 6) {
        lv_label_set_text(_page_title, "Function 7 - AI Glasses Settings");
        lv_obj_add_flag(_page_badge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_page_hint, LV_OBJ_FLAG_HIDDEN);
        setTranslateControlsVisible(false);
        setSttControlsVisible(false);
        setFollowControlsVisible(false);
        setSettingsControlsVisible(true);
    } else {
        lv_label_set_text(_page_title, title);
        lv_label_set_text(_page_badge_label, badge_text);
        lv_label_set_text(_page_hint, hint);
        lv_obj_clear_flag(_page_badge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_page_hint, LV_OBJ_FLAG_HIDDEN);
        setTranslateControlsVisible(false);
        setSttControlsVisible(false);
        setFollowControlsVisible(false);
        setSettingsControlsVisible(false);
    }

    if(_home_view) {
        lv_obj_add_flag(_home_view, LV_OBJ_FLAG_HIDDEN);
    }
    if(_page_view) {
        lv_obj_clear_flag(_page_view, LV_OBJ_FLAG_HIDDEN);
    }
}

void ScrollTestApp::createScrollItem(lv_obj_t *item, uint16_t index)
{
    char text[8];

    lv_obj_set_style_radius(item, 12, 0);
    lv_obj_set_style_bg_color(item, kBgColor, 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(item, 3, 0);
    lv_obj_set_style_border_color(item, kGreen, 0);
    lv_obj_set_style_shadow_width(item, 0, 0);
    lv_obj_set_style_outline_width(item, 0, 0);
    lv_obj_set_style_pad_all(item, 0, 0);

    lv_obj_t *index_label = lv_label_create(item);
    std::snprintf(text, sizeof(text), "%u", index + 1);
    lv_label_set_text(index_label, text);
    lv_obj_set_style_text_color(index_label, kTextWhite, 0);
    lv_obj_set_style_text_font(index_label, &lv_font_montserrat_34, 0);
    lv_obj_align(index_label, LV_ALIGN_CENTER, 0, -8);

    lv_obj_t *caption = lv_label_create(item);
    lv_label_set_text(caption, "ITEM");
    lv_obj_set_style_text_color(caption, kGreenSoft, 0);
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_12, 0);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, 28);
}

void ScrollTestApp::scaleScrollItem(lv_obj_t *item, float scale)
{
    lv_obj_t *index_label = lv_obj_get_child(item, 0);
    lv_obj_t *caption = lv_obj_get_child(item, 1);
    lv_coord_t border_width = scale > 0.90f ? 4 : 3;
    lv_opa_t border_opa = (lv_opa_t)(110 + (255 - 110) * scale);
    lv_opa_t text_opa = (lv_opa_t)(120 + (255 - 120) * scale);
    lv_opa_t caption_opa = (lv_opa_t)(90 + (210 - 90) * scale);

    lv_obj_set_style_border_width(item, border_width, 0);
    lv_obj_set_style_border_opa(item, border_opa, 0);
    lv_obj_set_style_text_opa(index_label, text_opa, 0);
    lv_obj_set_style_text_opa(caption, caption_opa, 0);
    lv_obj_set_style_border_color(item, scale > 0.90f ? kGreen : kGreenDim, 0);
}

void ScrollTestApp::onScrollContentChanged(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    if(app) {
        app->updateTitleDisplay();
    }
}

void ScrollTestApp::updateTitleDisplay(void)
{
    if((_scroll_bar.children == nullptr) || (_scroll_bar.title_labels == nullptr) || (_scroll_bar.selected >= kItemCount)) {
        return;
    }

    for(uint16_t i = 0; i < kItemCount; ++i) {
        if(_title_labels[i]) {
            lv_obj_add_flag(_title_labels[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_text_font(_title_labels[i], &lv_font_montserrat_16, 0);
        }
    }

    lv_obj_t *label = _title_labels[_scroll_bar.selected];
    lv_obj_t *item = _scroll_bar.children[_scroll_bar.selected];
    if((label != nullptr) && (item != nullptr)) {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_align_to(label, item, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
        lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    }
}

void ScrollTestApp::onScrollItemClicked(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    lv_obj_t *target = lv_event_get_target(e);

    if(app == nullptr) {
        return;
    }

    for(uint16_t i = 0; i < kItemCount; ++i) {
        if(app->_scroll_bar.children[i] == target) {
            app->showPage(i);
            return;
        }
    }
}

void ScrollTestApp::onPageBackClicked(lv_event_t *e)
{
    auto *app = static_cast<ScrollTestApp *>(lv_event_get_user_data(e));
    if(app) {
        app->showHome();
    }
}
