#include "a2_strdup.h"
#include "pager_layer.h"

struct PagerLayer {
	Layer *layer;
	uint8_t index;
	uint8_t count;
	GTextLayoutCacheRef cache;
};

static const GSize outer_size = { 7, 7 };
static const uint16_t outer_radius = 3;
static const int16_t padding = 5;
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

	return pager_layer;
}

void pager_layer_destroy(PagerLayer *pager_layer) {
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
			pager_layer->cache
		);
	} else {
		GRect rect = GRect(0, 0, outer_size.w * count + padding * (count - 1), outer_size.h);
		grect_align(&rect, &bounds, GAlignCenter, false);

		GRect circle_rect = rect;
		circle_rect.size.w = outer_size.w;
		const int16_t dx = outer_size.w + padding;

		for (uint8_t i = 0; i < count; i++) {
			if (i == index) {
				graphics_fill_rect(ctx, circle_rect, outer_radius, GCornersAll);
			} else {
				graphics_draw_round_rect(ctx, circle_rect, outer_radius);
			}

			circle_rect.origin.x += dx;
		}
	}
}