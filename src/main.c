#include <pebble.h>

#include "card_layer.h"
#include "defines.h"
#include "error_window.h"
#include "pager_layer.h"
#include "pebble_process_info.h"
#include "refresh_layer.h"

extern const PebbleProcessInfo __pbl_app_info;

static CardLayer *card_layer;
static PagerLayer *pager_layer;
static RefreshLayer *refresh_layer;
static Window *window;

static bool updating = false;
static uint8_t current_page = 0;

static AppTimer *finish_update_timer;
static void finish_update_timer_callback(void *data);

static void init(void);
static void deinit(void);
static void upgrade(void);

static void window_load(Window *window);
static void window_unload(Window *window);
static void window_click_config_provider(void *context);
static void window_down_click_handler(ClickRecognizerRef recognizer, void *context);
static void window_up_click_handler(ClickRecognizerRef recognizer, void *context);
static void window_select_click_handler(ClickRecognizerRef recognizer, void *context);

static void app_message_init(void);
static void app_message_send_fetch_data(void);
static void app_message_inbox_dropped(AppMessageResult reason, void *context);
static void app_message_inbox_received(DictionaryIterator *iterator, void *context);
static void app_message_outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void *context);
static void app_message_outbox_sent(DictionaryIterator *iterator, void *context);

static void card_layer_init(void);
static void card_layer_deinit(void);

static void pager_layer_init(void);
static void pager_layer_deinit(void);

static void refresh_layer_init(void);
static void refresh_layer_deinit(void);

static void update_visible_layers(void);

int main(void) {
    init();
    app_event_loop();
    deinit();
}

static void init(void) {
    upgrade();

    app_message_init();
    error_window_init();
    refresh_layer_global_init();

    window = window_create();
    window_set_background_color(window, GColorBlack);
    window_set_click_config_provider(window, window_click_config_provider);
    window_set_fullscreen(window, true);

    static const WindowHandlers window_handlers = {
        .load = window_load,
        .unload = window_unload,
    };
    window_set_window_handlers(window, window_handlers);
    window_stack_push(window, true);
}

static void deinit(void) {
    window_destroy(window);

    error_window_deinit();
    refresh_layer_global_deinit();

    light_enable(false);
}

static void upgrade(void) {
    Version saved_version = { 0, 0 };
    persist_read_data(STORAGE_APP_VERSION, &saved_version, sizeof(saved_version));

    if (saved_version.major < 3) {
        for (uint32_t i = 0; i < 5; i++) {
            persist_delete(i);
        }
    }

    Version current_version = __pbl_app_info.process_version;
    persist_write_data(STORAGE_APP_VERSION, &current_version, sizeof(current_version));
}

static void window_load(Window *window) {
    card_layer_init();
    pager_layer_init();
    refresh_layer_init();

    update_visible_layers();
}

static void window_unload(Window *window) {
    card_layer_deinit();
    pager_layer_deinit();
    refresh_layer_deinit();
}

static void window_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_DOWN, window_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, window_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, window_select_click_handler);
}

/*
 * Advance to the next card when down is pressed
 */
static void window_down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (updating) return;

    uint8_t num_cards = 0;
    persist_read_data(STORAGE_NUMBER_OF_CARDS, &num_cards, sizeof(num_cards));

    if (num_cards > 0) {
        current_page = (current_page + 1) % num_cards;
        update_visible_layers();
    }
}

/*
 * Go to the previous card when up is pressed
 */
static void window_up_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (updating) return;

    uint8_t num_cards = 0;
    persist_read_data(STORAGE_NUMBER_OF_CARDS, &num_cards, sizeof(num_cards));
    
    if (num_cards > 0) {
        current_page = (current_page - 1) % num_cards;
        update_visible_layers();
    }
}

static void disable_force_light(void *data) {
    light_enable(false);
}

/*
 * Turn backlight on for 30 seconds when select button is pressed
 */
static void window_select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (updating) return;

    light_enable(true);
    app_timer_register(30000, disable_force_light, NULL);
}

static void app_message_init(void) {
    app_message_register_inbox_dropped(app_message_inbox_dropped);
    app_message_register_inbox_received(app_message_inbox_received);
    app_message_register_outbox_failed(app_message_outbox_failed);
    app_message_register_outbox_sent(app_message_outbox_sent);
    app_message_open(app_message_inbox_size_maximum(), 256);
}

static void app_message_inbox_dropped(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s <- 0x%X", __PRETTY_FUNCTION__, reason);
}

/*
 * Read new barcode data sent from config page for skunk in pebble app.
 * Save this information to persistant storage
 */
static void app_message_read_first_payload(DictionaryIterator *dict) {
    Tuple *tuple = dict_read_first(dict);
    if (!tuple) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[MAIN] dict_read_first -> NULL");
        return;
    }

    while (tuple) {
        uint32_t int32_key = 0;
        uint32_t time_key = 0;
        switch (tuple->key) {
            case KEY_NUMBER_OF_CARDS:
                int32_key = STORAGE_NUMBER_OF_CARDS;
                break;
            case KEY_REWARDS_UPDATED_AT:
                time_key = STORAGE_REWARDS_UPDATED_AT;
                break;
        }

        if (int32_key) {
            uint8_t value = tuple->value->int32;
            persist_write_data(int32_key, &value, sizeof(value));
        } else if (time_key) {
            time_t value = tuple->value->int32;
            persist_write_data(time_key, &value, sizeof(value));
        }

        tuple = dict_read_next(dict);
    }
}

/*
 * Read changed barcode data sent from config page for skunk in pebble app.
 * Save this information to persistant storage
 */
static void app_message_read_card_payload(DictionaryIterator *dict, int32_t card_index) {
    Tuple *tuple = dict_read_first(dict);
    if (!tuple) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[CARD %ld] dict_read_first -> NULL", card_index);
        return;
    }

    while (tuple) {
        uint32_t cstr_key = 0;
        uint32_t data_key = 0;
        switch (tuple->key) {
            case KEY_CARD_BARCODE_DATA:
                data_key = STORAGE_CARD_VALUE(BARCODE_DATA, card_index);
                break;
            case KEY_CARD_NAME:
                cstr_key = STORAGE_CARD_VALUE(NAME, card_index);
                break;
            case KEY_CARD_VALUE:
                cstr_key = STORAGE_CARD_VALUE(VALUE, card_index);
                break;
        }

        if (cstr_key) {
            persist_write_string(cstr_key, tuple->value->cstring);
        } else if (data_key) {
            persist_write_data(data_key, tuple->value->data, tuple->length);
        }

        tuple = dict_read_next(dict);
    }
}

/*
 * Determines what to do with data received from skunk config page
 */
static void app_message_inbox_received(DictionaryIterator *dict, void *context) {
    Tuple *pushing_data = dict_find(dict, KEY_PUSHING_DATA);
    if (pushing_data) {
        app_message_send_fetch_data();
        return;
    }

    Tuple *error_tpl = dict_find(dict, KEY_ERROR);
    if (error_tpl) {
        char *string = error_tpl->value->cstring;
        APP_LOG(APP_LOG_LEVEL_ERROR, "%s <- \"%s\"", __PRETTY_FUNCTION__, string);
        error_window_push(string);
        return;
    }

    Tuple *card_index_tpl = dict_find(dict, KEY_CARD_INDEX);
    if (card_index_tpl) {
        int32_t card_index = card_index_tpl->value->int32;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s <- CARD %ld", __PRETTY_FUNCTION__, card_index);
        app_message_read_card_payload(dict, card_index);
    } else {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s <- MAIN", __PRETTY_FUNCTION__);
        app_message_read_first_payload(dict);
    }

    if (finish_update_timer) app_timer_cancel(finish_update_timer);
    finish_update_timer = app_timer_register(1000, finish_update_timer_callback, NULL);
}

static void app_message_outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s <- 0x%X", __PRETTY_FUNCTION__, reason);

    if (!updating) return;
    updating = false;
    update_visible_layers();
}

static void app_message_outbox_sent(DictionaryIterator *iterator, void *context) {
    finish_update_timer = app_timer_register(15 * 1000, finish_update_timer_callback, NULL);
}

static void app_message_send_fetch_data(void) {
    updating = true;
    update_visible_layers();

    DictionaryIterator *dict;
    app_message_outbox_begin(&dict);
    dict_write_uint8(dict, KEY_FETCH_DATA, 1);
    app_message_outbox_send();
}

static void card_layer_init(void) {
    card_layer = card_layer_create(GRect(0, 0, PEBBLE_WIDTH, 140)); // TODO: fix 140 magic number
    card_layer_set_index(card_layer, 0);

    Layer *root_layer = window_get_root_layer(window);
    layer_add_child(root_layer, card_layer_get_layer(card_layer));
}

static void card_layer_deinit(void) {
    card_layer_destroy(card_layer);
}

static void pager_layer_init(void) {
    pager_layer = pager_layer_create(GRect(0, PEBBLE_HEIGHT - 18, PEBBLE_WIDTH, 7)); // TODO: fix magic numbers
    pager_layer_set_values(pager_layer, 0, 1);

    Layer *root_layer = window_get_root_layer(window);
    layer_add_child(root_layer, pager_layer_get_layer(pager_layer));
}

static void pager_layer_deinit(void) {
    pager_layer_destroy(pager_layer);
}

static void refresh_layer_init(void) {
    refresh_layer = refresh_layer_create(GRect(4, 50, 136, 66)); // TODO: fix magic numbers

    Layer *base_layer = refresh_layer_get_layer(refresh_layer);
    layer_set_hidden(base_layer, true);

    Layer *root_layer = window_get_root_layer(window);
    layer_add_child(root_layer, base_layer);
}

static void refresh_layer_deinit(void) {
    refresh_layer_destroy(refresh_layer);
}

static void update_visible_layers(void) {
    const bool refresh_layer_hidden = !updating && persist_exists(STORAGE_NUMBER_OF_CARDS) && persist_read_int(STORAGE_NUMBER_OF_CARDS) != 0;
    const bool cards_layer_hidden = updating || !refresh_layer_hidden;
    const bool pager_layer_hidden = updating;

    uint8_t num_cards = 0;
    persist_read_data(STORAGE_NUMBER_OF_CARDS, &num_cards, sizeof(num_cards));

    if (current_page > num_cards - 1) {
        current_page = num_cards - 1;
    }

    if (!cards_layer_hidden) {
        card_layer_set_index(card_layer, current_page - 1);
    }

    if (!pager_layer_hidden) {
        pager_layer_set_values(pager_layer, current_page, num_cards);
    }

    refresh_layer_set_updating(refresh_layer, updating);

    layer_set_hidden(refresh_layer_get_layer(refresh_layer), refresh_layer_hidden);
    layer_set_hidden(card_layer_get_layer(card_layer), cards_layer_hidden);
    layer_set_hidden(pager_layer_get_layer(pager_layer), pager_layer_hidden);
}

static void finish_update_timer_callback(void *data) {
    finish_update_timer = NULL;

    if (updating) {
        updating = false;
        update_visible_layers();
    }
}
