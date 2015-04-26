#include "refresh_layer.h"

static GBitmap *error_bitmap;
static GBitmap *refresh_bitmap;
static const char *const refresh_text = "Please open\nsettings.";
static const char *const refresh_text_updating = "Updating...";
static const int16_t text_layer_height = 40;

struct RefreshLayer {
    Layer *layer;
    BitmapLayer *icon_layer;
    TextLayer *text_layer;
};

void refresh_layer_global_init(void) {
    refresh_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REFRESH);
    error_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ERROR);
}

void refresh_layer_global_deinit(void) {
    gbitmap_destroy(refresh_bitmap);
    gbitmap_destroy(error_bitmap);
}

RefreshLayer *refresh_layer_create(GRect frame) {
    static const size_t RefreshLayer_size = sizeof(RefreshLayer);
    RefreshLayer *refresh_layer = malloc(RefreshLayer_size);
    memset(refresh_layer, 0, RefreshLayer_size);

    refresh_layer->layer = layer_create(frame);

    GRect icon_layer_frame = GRect(0, 0, frame.size.w, frame.size.h - text_layer_height);
    refresh_layer->icon_layer = bitmap_layer_create(icon_layer_frame);
    bitmap_layer_set_bitmap(refresh_layer->icon_layer, error_bitmap);
    bitmap_layer_set_compositing_mode(refresh_layer->icon_layer, GCompOpAssignInverted);
    layer_add_child(refresh_layer->layer, (Layer *)refresh_layer->icon_layer);

    GRect text_layer_frame = GRect(0, frame.size.h - text_layer_height, frame.size.w, text_layer_height);
    refresh_layer->text_layer = text_layer_create(text_layer_frame);
    text_layer_set_background_color(refresh_layer->text_layer, GColorBlack);
    text_layer_set_font(refresh_layer->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(refresh_layer->text_layer, refresh_text);
    text_layer_set_text_alignment(refresh_layer->text_layer, GTextAlignmentCenter);
    text_layer_set_text_color(refresh_layer->text_layer, GColorWhite);
    layer_add_child(refresh_layer->layer, (Layer *)refresh_layer->text_layer);

    return refresh_layer;
}

void refresh_layer_destroy(RefreshLayer *refresh_layer) {
    layer_destroy(refresh_layer->layer);
    bitmap_layer_destroy(refresh_layer->icon_layer);
    text_layer_destroy(refresh_layer->text_layer);
    free(refresh_layer);
}

void refresh_layer_set_updating(RefreshLayer *refresh_layer, bool updating) {
    bitmap_layer_set_bitmap(refresh_layer->icon_layer, updating ? refresh_bitmap : error_bitmap);
    text_layer_set_text(refresh_layer->text_layer, updating ? refresh_text_updating : refresh_text);
}

Layer *refresh_layer_get_layer(RefreshLayer *refresh_layer) {
    return refresh_layer->layer;
}
