#pragma once

#include <pebble.h>

struct PagerLayer;
typedef struct PagerLayer PagerLayer;

extern PagerLayer *pager_layer_create(GRect frame);
extern void pager_layer_destroy(PagerLayer *pager_layer);
extern void pager_layer_set_values(PagerLayer *pager_layer, uint8_t index, uint8_t count);

// PagerLayer CANNOT be cast to Layer
extern Layer *pager_layer_get_layer(PagerLayer *pager_layer);
