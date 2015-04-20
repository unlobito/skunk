#include "a2_strdup.h"
#include "card_layer.h"
#include "defines.h"
#include "tobinstr.h"

#define BARCODE_SUB_BITMAP_COUNT 8

struct CardLayer {
	Layer *layer;
	int barcode_height;
	int barcode_width;
	uint8_t *barcode_data;
	GBitmap *barcode;
	char *balance_text;
	TextLayer *balance_text_layer;
	char *name_text;
	TextLayer *name_text_layer;
};

static void background_update_proc(Layer *layer, GContext* ctx);

CardLayer *card_layer_create(GRect frame) {
	static const size_t CardLayer_size = sizeof(CardLayer);
	CardLayer *card_layer = malloc(CardLayer_size);
	memset(card_layer, 0, CardLayer_size);

	card_layer->layer = layer_create_with_data(frame, sizeof(CardLayer *));
	layer_set_update_proc(card_layer->layer, background_update_proc);
	*(CardLayer **)layer_get_data(card_layer->layer) = card_layer;

	card_layer->name_text_layer = text_layer_create(GRect(0, 0, 144, 22));
	text_layer_set_background_color(card_layer->name_text_layer, GColorBlack);
	text_layer_set_font(card_layer->name_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_overflow_mode(card_layer->name_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_text_alignment(card_layer->name_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(card_layer->name_text_layer, GColorWhite);
	layer_add_child(card_layer->layer, (Layer *)card_layer->name_text_layer);

	return card_layer;
}

void card_layer_destroy(CardLayer *card_layer) {
	layer_destroy(card_layer->layer);
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
	CardLayer *card_layer = *(CardLayer **)layer_get_data(layer);
	if (!card_layer->barcode_data) return;

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

	graphics_context_set_fill_color(ctx, GColorBlack);

	int8_t raw_x, raw_y, point_x, point_y;
	raw_x = 0;
	raw_y = 0;

	for (int8_t i = 2; i < (2 + card_layer->barcode_width * card_layer->barcode_height / 8 ); i++) {
		char block[9];
		tobinstr(card_layer->barcode_data[i], 8, block);

		for (int8_t p = 7; p >= 0; p--) {
			if (block[p] == 49) {
				point_x = ( 72 - card_layer->barcode_width ) + raw_x * 2;
				point_y = ( 84 - card_layer->barcode_height ) + raw_y * 2;

				graphics_draw_pixel(ctx, GPoint(point_x, point_y));
				graphics_draw_pixel(ctx, GPoint(point_x + 1, point_y));
				graphics_draw_pixel(ctx, GPoint(point_x + 1, point_y + 1));
				graphics_draw_pixel(ctx, GPoint(point_x, point_y + 1));
			}

			raw_x++;
			if (raw_x == card_layer->barcode_width ) {
				raw_x = 0;
				raw_y++;
			}
		}

		memset(block, 0, 9);
	}
}

bool card_layer_set_index(CardLayer *card_layer, uint8_t index) {
	index = index + 1;

	uint8_t num_cards = 0;
	persist_read_data(STORAGE_NUMBER_OF_CARDS, &num_cards, sizeof(num_cards));
	if (index >= num_cards) return false;

	// BARCODE DATA

	if (card_layer->barcode) gbitmap_destroy(card_layer->barcode);
	if (card_layer->barcode_data) free(card_layer->barcode_data);

	const uint32_t barcode_data_key = STORAGE_CARD_VALUE(BARCODE_DATA, index);
	int barcode_data_size = persist_get_size(barcode_data_key);
	card_layer->barcode_data = malloc(barcode_data_size);
	persist_read_data(barcode_data_key, card_layer->barcode_data, barcode_data_size);

	GBitmap *barcode = gbitmap_create_with_data(card_layer->barcode_data);
	card_layer->barcode = barcode;

	card_layer->barcode_width = card_layer->barcode_data[0];
	card_layer->barcode_height = card_layer->barcode_data[1];

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
