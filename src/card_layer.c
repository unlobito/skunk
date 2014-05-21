#include "a2_strdup.h"
#include "card_layer.h"
#include "defines.h"

#define BARCODE_SUB_BITMAP_COUNT 8

struct CardLayer {
	Layer *layer;
	uint8_t *barcode_data;
	GBitmap *barcode;
	GBitmap *barcode_sub_bitmaps[BARCODE_SUB_BITMAP_COUNT];
	char *balance_text;
	TextLayer *balance_text_layer;
	char *name_text;
	TextLayer *name_text_layer;
};

static const GRect barcode_rect = {{1, 41}, {142, 64}};
static void background_update_proc(Layer *layer, GContext* ctx);

CardLayer *card_layer_create(GRect frame) {
	static const size_t CardLayer_size = sizeof(CardLayer);
	CardLayer *card_layer = malloc(CardLayer_size);
	memset(card_layer, 0, CardLayer_size);

	card_layer->layer = layer_create_with_data(frame, sizeof(CardLayer *));
	layer_set_update_proc(card_layer->layer, background_update_proc);
	*(CardLayer **)layer_get_data(card_layer->layer) = card_layer;

	card_layer->name_text_layer = text_layer_create(GRect(4, -2, 136, 22));
	text_layer_set_background_color(card_layer->name_text_layer, GColorBlack);
	text_layer_set_font(card_layer->name_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_overflow_mode(card_layer->name_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_text_alignment(card_layer->name_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(card_layer->name_text_layer, GColorWhite);
	layer_add_child(card_layer->layer, (Layer *)card_layer->name_text_layer);

	card_layer->balance_text_layer = text_layer_create(GRect(4, 16, 136, 18));
	text_layer_set_background_color(card_layer->balance_text_layer, GColorBlack);
	text_layer_set_font(card_layer->balance_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(card_layer->balance_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(card_layer->balance_text_layer, GColorWhite);
	layer_add_child(card_layer->layer, (Layer *)card_layer->balance_text_layer);

	return card_layer;
}

void card_layer_destroy(CardLayer *card_layer) {
	layer_destroy(card_layer->layer);
	for (int8_t i = 0; i < BARCODE_SUB_BITMAP_COUNT; i++) {
		if (card_layer->barcode_sub_bitmaps[i]) {
			gbitmap_destroy(card_layer->barcode_sub_bitmaps[i]);
		}
	}
	if (card_layer->barcode) gbitmap_destroy(card_layer->barcode);
	if (card_layer->barcode_data) free(card_layer->barcode_data);
	if (card_layer->balance_text) free(card_layer->balance_text);
	text_layer_destroy(card_layer->balance_text_layer);
	if (card_layer->name_text) free(card_layer->name_text);
	text_layer_destroy(card_layer->name_text_layer);
	free(card_layer);
}

Layer *card_layer_get_layer(CardLayer *card_layer) {
	return card_layer->layer;
}

static void background_update_proc(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, grect_crop(barcode_rect, -1), 0, GCornerNone);

	CardLayer *card_layer = *(CardLayer **)layer_get_data(layer);
	if (!card_layer->barcode_data) return;

	GRect rect = barcode_rect;
	rect.size.h = 8;
	for (int8_t i = 0; i < BARCODE_SUB_BITMAP_COUNT; i++) {
		GBitmap *sub_bitmap = card_layer->barcode_sub_bitmaps[i];
		graphics_draw_bitmap_in_rect(ctx, sub_bitmap, rect);
		rect.origin.y += 8;
	}
}

bool card_layer_set_index(CardLayer *card_layer, uint8_t index) {
	uint8_t num_cards = 0;
	persist_read_data(STORAGE_NUMBER_OF_CARDS, &num_cards, sizeof(num_cards));
	if (index >= num_cards) return false;

	// BALANCE

	if (card_layer->balance_text) free(card_layer->balance_text);

	const uint32_t balance_key = STORAGE_CARD_VALUE(BALANCE, index);
	char balance_buffer[32];
	int balance_bytes_read = persist_read_string(balance_key, balance_buffer, sizeof(balance_buffer));
	balance_buffer[MAX(0, balance_bytes_read)] = '\0';

	card_layer->balance_text = strdup(balance_buffer);
	text_layer_set_text(card_layer->balance_text_layer, card_layer->balance_text);

	// BARCODE DATA

	for (int8_t i = 0; i < BARCODE_SUB_BITMAP_COUNT; i++) {
		if (card_layer->barcode_sub_bitmaps[i]) {
			gbitmap_destroy(card_layer->barcode_sub_bitmaps[i]);
		}
	}
	if (card_layer->barcode) gbitmap_destroy(card_layer->barcode);
	if (card_layer->barcode_data) free(card_layer->barcode_data);

	const uint32_t barcode_data_key = STORAGE_CARD_VALUE(BARCODE_DATA, index);
	int barcode_data_size = persist_get_size(barcode_data_key);
	card_layer->barcode_data = malloc(barcode_data_size);
	persist_read_data(barcode_data_key, card_layer->barcode_data, barcode_data_size);

	GBitmap *barcode = gbitmap_create_with_data(card_layer->barcode_data);
	card_layer->barcode = barcode;

	GRect sub_rect = barcode->bounds;
	sub_rect.size.h = 1;
	for (int8_t i = 0; i < BARCODE_SUB_BITMAP_COUNT; i++) {
		sub_rect.origin.y = i;
		card_layer->barcode_sub_bitmaps[i] = gbitmap_create_as_sub_bitmap(barcode, sub_rect);
	}

	layer_mark_dirty(card_layer->layer);

	// NAME

	if (card_layer->name_text) free(card_layer->name_text);

	const uint32_t name_key = STORAGE_CARD_VALUE(NAME, index);
	char name_buffer[256];
	int name_bytes_read = persist_read_string(name_key, name_buffer, sizeof(name_buffer));
	name_buffer[MAX(0, name_bytes_read)] = '\0';

	card_layer->name_text = strdup(name_buffer);
	text_layer_set_text(card_layer->name_text_layer, card_layer->name_text);

	return true;
}
