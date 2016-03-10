#include "a2_strdup.h"
#include "error_window.h"
#include "defines.h"

static GBitmap *error_bitmap;
static Window *window;
static BitmapLayer *icon_layer;
static char *text;
static TextLayer *text_layer;

static void window_load(Window *window);
static void window_click_config_provider(void *context);
static void window_select_click_handler(ClickRecognizerRef recognizer, void *context);

void error_window_init(void) {
    error_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ERROR);

    window = window_create();
    window_set_background_color(window, GColorBlack);
    window_set_click_config_provider(window, window_click_config_provider);

    static const WindowHandlers window_handlers = {
        .load = window_load,
    };
    window_set_window_handlers(window, window_handlers);
}

void error_window_deinit(void) {
    gbitmap_destroy(error_bitmap);
    bitmap_layer_destroy(icon_layer);
    if (text) free(text);
    text_layer_destroy(text_layer);
    window_destroy(window);
}

bool error_window_push(char *error_text) {
    if (window && window_stack_contains_window(window)) {
        return false;
    }

    if (text) free(text);
    text = strdup(error_text);
    if (text_layer) text_layer_set_text(text_layer, text);
    window_stack_push(window, true);

    return true;
}

bool error_window_is_visible(void) {
    return window != NULL;
}

static void window_load(Window *window) {
    GRect icon_layer_frame = GRect(61, 60, 21, 19);
    icon_layer = bitmap_layer_create(icon_layer_frame);
    bitmap_layer_set_bitmap(icon_layer, error_bitmap);
    bitmap_layer_set_compositing_mode(icon_layer, GCompOpAssignInverted);

    GRect text_layer_frame = GRect(TEXT_MARGIN, 83, 136, 50);
    text_layer = text_layer_create(text_layer_frame);
    text_layer_set_background_color(text_layer, GColorBlack);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_text_color(text_layer, GColorWhite);
    if (text) text_layer_set_text(text_layer, text);

    Layer *root_layer = window_get_root_layer(window);
    layer_add_child(root_layer, (Layer *)icon_layer);
    layer_add_child(root_layer, (Layer *)text_layer);
}

static void window_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, window_select_click_handler);
}

static void window_select_click_handler(ClickRecognizerRef recognizer, void *context) {
    window_stack_remove(window, true);
}
