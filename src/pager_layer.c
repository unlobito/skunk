#include "pager_layer.h"
#include "defines.h"

struct PagerLayer {
    Layer *layer;
    uint8_t index;
    uint8_t count;
    /*interval use only: GTextLayoutCacheRef cache; */
#ifdef PBL_ROUND
    GTextAttributes *attributes;
#endif
};

static void background_update_proc(Layer *layer, GContext* ctx);

PagerLayer *pager_layer_create(GRect frame) {
    static const size_t PagerLayer_size = sizeof(PagerLayer);
    PagerLayer *pager_layer = malloc(PagerLayer_size);
    memset(pager_layer, 0, PagerLayer_size);

    pager_layer->layer = layer_create_with_data(frame, sizeof(PagerLayer *));
    layer_set_update_proc(pager_layer->layer, background_update_proc);
    *(PagerLayer **)layer_get_data(pager_layer->layer) = pager_layer;

    pager_layer->index = 0;
    pager_layer->count = 0;
#ifdef PBL_ROUND
    pager_layer->attributes = graphics_text_attributes_create();
    graphics_text_attributes_enable_screen_text_flow(pager_layer->attributes, TEXT_MARGIN);
#endif

    return pager_layer;
}

void pager_layer_destroy(PagerLayer *pager_layer) {
#ifdef PBL_ROUND
    graphics_text_attributes_destroy(pager_layer->attributes);
#endif
    layer_destroy(pager_layer->layer);
    free(pager_layer);
}

void pager_layer_set_values(PagerLayer *pager_layer, uint8_t index, uint8_t count) {
    pager_layer->index = index;
    pager_layer->count = count;
    layer_mark_dirty(pager_layer->layer);
}

Layer *pager_layer_get_layer(PagerLayer *pager_layer) {
    return pager_layer->layer;
}

static void background_update_proc(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_text_color(ctx, GColorWhite);

    const GRect bounds = layer_get_bounds(layer);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, GColorWhite);

    PagerLayer *pager_layer = *(PagerLayer **)layer_get_data(layer);
    const uint8_t index = pager_layer->index;
    const uint8_t count = pager_layer->count;

    if (count > 8) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "page %d/%d", index + 1, count);
        graphics_draw_text(
                           ctx,
                           buffer,
                           fonts_get_system_font(FONT_KEY_GOTHIC_18),
                           bounds,
                           GTextOverflowModeFill,
                           GTextAlignmentCenter,
#ifdef PBL_ROUND
                           pager_layer->attributes
#else
                           NULL //pager_layer->cache (always NULL for third-party apps)
#endif
                           );
    } else {
        int16_t y = bounds.origin.y + bounds.size.h / 2;
#ifdef PBL_ROUND
        y -= 4; // Magic number to avoid round screen border
#endif
        int16_t x = (bounds.origin.x + bounds.size.w - PAGER_LAYER_CIRCLE_DISTANCE * (count-1)) / 2;
        for (uint8_t i = 0; i < count; i++) {
            if (i == index) {
                graphics_fill_circle(ctx, GPoint(x, y), PAGER_LAYER_CIRCLE_RADIUS);
            } else {
                graphics_draw_circle(ctx, GPoint(x, y), PAGER_LAYER_CIRCLE_RADIUS);
            }
            x += PAGER_LAYER_CIRCLE_DISTANCE;
        }
    }
}
