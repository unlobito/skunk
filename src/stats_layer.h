#pragma once

#include <pebble.h>

struct StatsLayer;
typedef struct StatsLayer StatsLayer;

extern void stats_layer_global_init(void);
extern void stats_layer_global_deinit(void);

extern StatsLayer *stats_layer_create(GRect frame);
extern void stats_layer_destroy(StatsLayer *stats_layer);
extern void stats_layer_update(StatsLayer *stats_layer);

// StatsLayer CANNOT be cast to Layer
extern Layer *stats_layer_get_layer(StatsLayer *stats_layer);
