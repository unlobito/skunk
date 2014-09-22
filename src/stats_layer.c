#include "stats_layer.h"

#include "a2_strdup.h"
#include "defines.h"

static GBitmap *star_bitmap;
static GBitmap *drink_bitmap;

struct StatsLayer {
	Layer *layer;
	BitmapLayer *star_bitmap_layer;
	char *stars_text;
	TextLayer *stars_text_layer;
	BitmapLayer *drink_bitmap_layer;
	char *drinks_text;
	TextLayer *drinks_text_layer;
	char *updated_at_text;
	TextLayer *updated_at_text_layer;
};

extern void stats_layer_global_init(void) {
	star_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STAR);
	drink_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWARD);
}

extern void stats_layer_global_deinit(void) {
	gbitmap_destroy(star_bitmap);
	gbitmap_destroy(drink_bitmap);
}

StatsLayer *stats_layer_create(GRect frame) {
	static const size_t StatsLayer_size = sizeof(StatsLayer);
	StatsLayer *stats_layer = malloc(StatsLayer_size);
	memset(stats_layer, 0, StatsLayer_size);

	stats_layer->layer = layer_create(frame);

	stats_layer->star_bitmap_layer = bitmap_layer_create(GRect(12, 1, 21, 22));
	bitmap_layer_set_bitmap(stats_layer->star_bitmap_layer, star_bitmap);
	bitmap_layer_set_compositing_mode(stats_layer->star_bitmap_layer, GCompOpAssignInverted);
	layer_add_child(stats_layer->layer, (Layer *)stats_layer->star_bitmap_layer);

	stats_layer->drink_bitmap_layer = bitmap_layer_create(GRect(79, 0, 17, 23));
	bitmap_layer_set_bitmap(stats_layer->drink_bitmap_layer, drink_bitmap);
	bitmap_layer_set_compositing_mode(stats_layer->drink_bitmap_layer, GCompOpAssignInverted);
	layer_add_child(stats_layer->layer, (Layer *)stats_layer->drink_bitmap_layer);

	stats_layer->stars_text_layer = text_layer_create(GRect(39, -7, 30, 31));
	text_layer_set_background_color(stats_layer->stars_text_layer, GColorBlack);
	text_layer_set_font(stats_layer->stars_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_alignment(stats_layer->stars_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(stats_layer->stars_text_layer, GColorWhite);
	layer_add_child(stats_layer->layer, (Layer *)stats_layer->stars_text_layer);

	stats_layer->drinks_text_layer = text_layer_create(GRect(102, -7, 30, 31));
	text_layer_set_background_color(stats_layer->drinks_text_layer, GColorClear);
	text_layer_set_font(stats_layer->drinks_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_alignment(stats_layer->drinks_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(stats_layer->drinks_text_layer, GColorWhite);
	layer_add_child(stats_layer->layer, (Layer *)stats_layer->drinks_text_layer);

	stats_layer->updated_at_text_layer = text_layer_create(GRect(0, 29, 136, 50));
	text_layer_set_background_color(stats_layer->updated_at_text_layer, GColorClear);
	text_layer_set_font(stats_layer->updated_at_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(stats_layer->updated_at_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(stats_layer->updated_at_text_layer, GColorWhite);
	layer_add_child(stats_layer->layer, (Layer *)stats_layer->updated_at_text_layer);

	return stats_layer;
}

void stats_layer_destroy(StatsLayer *stats_layer) {
	layer_destroy(stats_layer->layer);
	bitmap_layer_destroy(stats_layer->star_bitmap_layer);
	free(stats_layer->stars_text);
	text_layer_destroy(stats_layer->stars_text_layer);
	bitmap_layer_destroy(stats_layer->drink_bitmap_layer);
	free(stats_layer->drinks_text);
	text_layer_destroy(stats_layer->drinks_text_layer);
	free(stats_layer->updated_at_text);
	text_layer_destroy(stats_layer->updated_at_text_layer);
	free(stats_layer);
}

Layer *stats_layer_get_layer(StatsLayer *stats_layer) {
	return stats_layer->layer;
}

static void stats_layer_set_text(TextLayer *text_layer, char **buffer_ptr, uint8_t value) {
	char buffer[8];
	snprintf(buffer, sizeof(buffer), "%hu", value);

	if (*buffer_ptr) free(*buffer_ptr);
	*buffer_ptr = strdup(buffer);
	text_layer_set_text(text_layer, *buffer_ptr);
}

static const char *time_format(void) {
	if (clock_is_24h_style())
		return "%d %b '%y @ %H:%M";
	else
		return "%d %b '%y @ %I:%M %p";
}

static void stats_layer_set_updated_at(StatsLayer *stats_layer, time_t updated_at) {
    char buffer[128];
    strcpy(buffer, "Updated:\n");

    if (updated_at > 0) {
	    size_t offset = strlen(buffer);
	    strftime(buffer + offset, sizeof(buffer) - offset, time_format(), localtime(&updated_at));
    } else {
    	strcat(buffer, "Never");
    }

    if (stats_layer->updated_at_text) free(stats_layer->updated_at_text);
    stats_layer->updated_at_text = strdup(buffer);
    text_layer_set_text(stats_layer->updated_at_text_layer, stats_layer->updated_at_text);
}

void stats_layer_update(StatsLayer *stats_layer) {
	uint8_t stars = 0;
	persist_read_data(STORAGE_REWARDS_STARS, &stars, sizeof(stars));
	stats_layer_set_text(stats_layer->stars_text_layer, &stats_layer->stars_text, stars);

	uint8_t drinks = 0;
	persist_read_data(STORAGE_REWARDS_DRINKS, &drinks, sizeof(drinks));
	stats_layer_set_text(stats_layer->drinks_text_layer, &stats_layer->drinks_text, drinks);

	time_t updated_at = 0;
	persist_read_data(STORAGE_REWARDS_UPDATED_AT, &updated_at, sizeof(updated_at));
	stats_layer_set_updated_at(stats_layer, updated_at);
}
