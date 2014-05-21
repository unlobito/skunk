#pragma once

#include <pebble.h>

extern void error_window_init(void);
extern void error_window_deinit(void);

extern bool error_window_push(char *error_text);
extern bool error_window_is_visible(void);
