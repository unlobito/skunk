#pragma once

#include <pebble.h>

struct RefreshLayer;
typedef struct RefreshLayer RefreshLayer;

extern void refresh_layer_global_init(void);
extern void refresh_layer_global_deinit(void);

extern RefreshLayer *refresh_layer_create(GRect frame);
extern void refresh_layer_destroy(RefreshLayer *refresh_layer);
extern void refresh_layer_set_updating(RefreshLayer *refresh_layer, bool updating);

// RefreshLayer CANNOT be cast to Layer
extern Layer *refresh_layer_get_layer(RefreshLayer *refresh_layer);
