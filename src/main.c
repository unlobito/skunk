#include <pebble.h>

#ifndef PEBBLE_HEIGHT
    #define PEBBLE_HEIGHT 168
#endif

#ifndef PEBBLE_WIDTH
    #define PEBBLE_WIDTH 144
#endif

static BitmapLayer *reward_bitmap_layer;
static BitmapLayer *star_bitmap_layer;

static char *label_text[4] = { NULL, NULL, NULL, NULL };
static const char *label_default_text[4] = { "0", "0", "$0", "" };
static const char *updating_text = "Updating...";
static const char *connection_error_text = "< ! > Error:\nNo Connection";

static GBitmap *reward;
static GBitmap *star;

static Layer *barcode_layer;

static TextLayer *starbucks_text_layer;
static TextLayer *text_layers[4] = { NULL, NULL, NULL, NULL };

static Window *window;

enum {
    KEY_REWARDS = 0,
    KEY_STARS   = 1,
    KEY_BALANCE = 2,
    KEY_STATUS  = 3,
    KEY_BARCODE = 4
};

#define reward_text_layer text_layers[KEY_REWARDS]
#define star_text_layer text_layers[KEY_STARS]
#define balance_text_layer text_layers[KEY_BALANCE]
#define status_text_layer text_layers[KEY_STATUS]

static char *a2_strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *dup = malloc(len);
    dup[len] = '\0';
    strcpy(dup, str);
    return dup;
}

static void free_text(int key) {
    if (label_text[key]) {
        free(label_text[key]);
        label_text[key] = NULL;
    }
}

static void set_text_from_dict(DictionaryIterator *iter, int key) {
    Tuple *tuple = dict_find(iter, key);
    if (!(tuple && tuple->type == TUPLE_CSTRING)) return;

    char *str = tuple->value->cstring;
    persist_write_string(key, str);

    free_text(key);
    label_text[key] = a2_strdup(str);
    text_layer_set_text(text_layers[key], label_text[key]);
}

static void load_text(int key, bool update_layer) {
    free_text(key);

    if (persist_exists(key)) {
        int size = persist_get_size(key);
        label_text[key] = malloc(size);
        persist_read_string(key, label_text[key], size);
    } else {
        label_text[key] = a2_strdup(label_default_text[key]);
    }

    if (update_layer) {
        text_layer_set_text(text_layers[key], label_text[key]);
    }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    set_text_from_dict(iter, KEY_REWARDS);
    set_text_from_dict(iter, KEY_STARS);
    set_text_from_dict(iter, KEY_BALANCE);
    set_text_from_dict(iter, KEY_STATUS);
    
    Tuple *barcode_tuple = dict_find(iter, KEY_BARCODE);
    if (barcode_tuple) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: barcode_tuple->length -> %d", __PRETTY_FUNCTION__, barcode_tuple->length);
        int size = persist_write_data(KEY_BARCODE, barcode_tuple->value->data, barcode_tuple->length);
        APP_LOG((size <= 0 ? APP_LOG_LEVEL_ERROR : APP_LOG_LEVEL_DEBUG), "%s: persist_write_data -> %d", __PRETTY_FUNCTION__, size);

        layer_mark_dirty(barcode_layer);
    }
}

static void out_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    static const int key = KEY_STATUS;
    free_text(key);

    label_text[key] = a2_strdup(connection_error_text);
    text_layer_set_text(text_layers[key], label_text[key]);
}

#define BARCODE_MIN_X (PEBBLE_WIDTH - 65)
#define BARCODE_MAX_X (PEBBLE_WIDTH - 1)
#define BARCODE_MIN_Y 1
#define BARCODE_MAX_Y (PEBBLE_HEIGHT - 1)

static void barcode_layer_update_proc(struct Layer *layer, GContext *ctx) {
    if (!persist_exists(KEY_BARCODE)) return;

    int size = persist_get_size(KEY_BARCODE);
    APP_LOG((size <= 0 ? APP_LOG_LEVEL_ERROR : APP_LOG_LEVEL_DEBUG), "%s: persist_get_size -> %d", __PRETTY_FUNCTION__, size);
    if (size <= 0) return;

    uint8_t buffer[size];
    size = persist_read_data(KEY_BARCODE, &buffer, size);
    APP_LOG((size <= 0 ? APP_LOG_LEVEL_ERROR : APP_LOG_LEVEL_DEBUG), "%s: persist_read_data -> %d", __PRETTY_FUNCTION__, size);
    if (size <= 0) return;

    graphics_context_set_fill_color(ctx, GColorWhite);
    GRect draw_rect = GRect(
        BARCODE_MIN_X - 1,
        BARCODE_MIN_Y - 1,
        ((BARCODE_MAX_X + 1) - (BARCODE_MIN_X - 1)),
        ((BARCODE_MAX_Y + 1) - (BARCODE_MIN_Y - 1))
    );
    graphics_fill_rect(ctx, draw_rect, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, GColorBlack);
    int16_t rect_y = 0;
    for (int16_t y = 0; y < size; y++) {
        uint8_t column = buffer[y];

        int16_t rect_h = (y == 0 || y == 14 || y == 28 || y == 43 || y == 57 || y == 71) ? 1 : 2;

        for (int16_t x = 0; x < 8; x++) {
            if (column & (1 << x)) {
                GRect rect = GRect(BARCODE_MIN_X + 8 * x, BARCODE_MIN_Y + rect_y, 8, rect_h);
                graphics_fill_rect(ctx, rect, 0, GCornerNone);
            }
        }

        rect_y += rect_h;
    }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    text_layer_set_text(status_text_layer, updating_text);
    app_message_outbox_send();
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
    starbucks_text_layer = text_layer_create(GRect(2, 0, 78, 26));
    text_layer_set_font(starbucks_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_overflow_mode(starbucks_text_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_background_color(starbucks_text_layer, GColorClear);
    text_layer_set_text_color(starbucks_text_layer, GColorWhite);
    text_layer_set_text(starbucks_text_layer, "Starbucks");

    reward_text_layer = text_layer_create(GRect(2, 96, 40, 30));
    text_layer_set_font(reward_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_overflow_mode(reward_text_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_alignment(reward_text_layer, GTextAlignmentRight);
    text_layer_set_background_color(reward_text_layer, GColorClear);
    text_layer_set_text_color(reward_text_layer, GColorWhite);
    text_layer_set_text(reward_text_layer, label_text[KEY_REWARDS]);

    star_text_layer = text_layer_create(GRect(2, 130, 40, 30));
    text_layer_set_font(star_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_overflow_mode(star_text_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_alignment(star_text_layer, GTextAlignmentRight);
    text_layer_set_background_color(star_text_layer, GColorClear);
    text_layer_set_text_color(star_text_layer, GColorWhite);
    text_layer_set_text(star_text_layer, label_text[KEY_STARS]);

    balance_text_layer = text_layer_create(GRect(2, 30, 74, 30));
    text_layer_set_font(balance_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_overflow_mode(balance_text_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_background_color(balance_text_layer, GColorClear);
    text_layer_set_text_color(balance_text_layer, GColorWhite);
    text_layer_set_text(balance_text_layer, label_text[KEY_BALANCE]);

    status_text_layer = text_layer_create(GRect(2, 60, 78, 30));
    text_layer_set_font(status_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_overflow_mode(status_text_layer, GTextOverflowModeWordWrap);
    text_layer_set_background_color(status_text_layer, GColorClear);
    text_layer_set_text_color(status_text_layer, GColorWhite);
    text_layer_set_text(status_text_layer, label_text[KEY_STATUS]);

    reward_bitmap_layer = bitmap_layer_create(GRect(44, 98, 30, 30));
    bitmap_layer_set_compositing_mode(reward_bitmap_layer, GCompOpAssignInverted);
    bitmap_layer_set_alignment(reward_bitmap_layer, GAlignCenter);
    bitmap_layer_set_bitmap(reward_bitmap_layer, reward);

    star_bitmap_layer = bitmap_layer_create(GRect(44, 133, 30, 30));
    bitmap_layer_set_compositing_mode(star_bitmap_layer, GCompOpAssignInverted);
    bitmap_layer_set_alignment(star_bitmap_layer, GAlignCenter);
    bitmap_layer_set_bitmap(star_bitmap_layer, star);

    barcode_layer = layer_create(GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT));
    layer_set_update_proc(barcode_layer, barcode_layer_update_proc);

    Layer *window_layer = window_get_root_layer(window);
    layer_add_child(window_layer, text_layer_get_layer(starbucks_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(reward_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(star_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(balance_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(status_text_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(reward_bitmap_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(star_bitmap_layer));
    layer_add_child(window_layer, barcode_layer);
}

static void window_unload(Window *window) {
    text_layer_destroy(starbucks_text_layer);
    text_layer_destroy(reward_text_layer);
    text_layer_destroy(star_text_layer);
    text_layer_destroy(balance_text_layer);
    text_layer_destroy(status_text_layer);
    bitmap_layer_destroy(reward_bitmap_layer);
    bitmap_layer_destroy(star_bitmap_layer);
    layer_destroy(barcode_layer);
}

static void app_message_init(void) {
    app_message_register_outbox_failed(out_failed_handler);
    app_message_register_inbox_received(in_received_handler);
    app_message_open(app_message_inbox_size_maximum() /* size_inbound */, 2 /* size_outbound */);
}

static void init(void) {
    reward = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWARD);
    star = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STAR);

    load_text(KEY_BALANCE, false);
    load_text(KEY_REWARDS, false);
    load_text(KEY_STARS, false);
    load_text(KEY_STATUS, false);

    window = window_create();
    window_set_fullscreen(window, true);
    window_set_background_color(window, GColorBlack);
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    window_stack_push(window, true);

    app_message_init();
}

static void deinit(void) {
    gbitmap_destroy(reward);
    gbitmap_destroy(star);

    free_text(KEY_BALANCE);
    free_text(KEY_REWARDS);
    free_text(KEY_STARS);
    free_text(KEY_STATUS);

    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
