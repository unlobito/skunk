#pragma once

#include <pebble.h>

struct CardLayer;
typedef struct CardLayer CardLayer;

extern CardLayer *card_layer_create(GRect frame);
extern void card_layer_destroy(CardLayer *card_layer);
extern bool card_layer_set_index(CardLayer *card_layer, uint8_t index);

// CardLayer CANNOT be cast to Layer
extern Layer *card_layer_get_layer(CardLayer *card_layer);
