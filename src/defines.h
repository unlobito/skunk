#pragma once

#ifdef PBL_ROUND
#define PEBBLE_HEIGHT 180
#define PEBBLE_WIDTH 180
#else
#define PEBBLE_HEIGHT 168
#define PEBBLE_WIDTH 144
#endif

#define TEXT_MARGIN 4
#define TEXT_HEIGHT 22

#define REFRESH_LAYER_HEIGHT 66
#define REFRESH_LAYER_TEXT_HEIGHT 22

#ifdef PBL_ROUND
#define CARD_LAYER_NAME_HEIGHT 30
#else
#define CARD_LAYER_NAME_HEIGHT 22
#endif
#define CARD_LAYER_VALUE_HEIGHT 22

#define PAGER_LAYER_HEIGHT 30
#define PAGER_LAYER_CIRCLE_DISTANCE 11
#define PAGER_LAYER_CIRCLE_RADIUS 3


#define IMG_HEADER_OFFSET 3
#define IMG_BIT_SIZE 8

#define ABS(A) ((A) < 0 ? -(A) : (A))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

enum {
    KEY_FETCH_DATA         =  0,
    KEY_PUSHING_DATA       =  1,

    KEY_NUMBER_OF_CARDS    = 10,
    KEY_REWARDS_UPDATED_AT = 13,

    KEY_CARD_INDEX         = 20,
    KEY_CARD_VALUE         = 21,
    KEY_CARD_BARCODE_DATA  = 22,
    KEY_CARD_NAME          = 23,

    KEY_ERROR              = 99,
};

enum {
    STORAGE_APP_VERSION               = 10, // Version

    STORAGE_NUMBER_OF_CARDS           = 11, // uint8_t
    STORAGE_REWARDS_UPDATED_AT        = 14, // time_t

    STORAGE_CARD_VALUE_OFFSET         = 0, // char[]
    STORAGE_CARD_BARCODE_DATA_OFFSET  = 1, // uint8_t[]
    STORAGE_CARD_NAME_OFFSET          = 2, // char[]
};

enum {
    BARCODE_MATRIX                     = 0,
    BARCODE_LINEAR                     = 1,
};


#define STORAGE_CARD_VALUE(NAME, INDEX) (20 + 10 * (INDEX) + (STORAGE_CARD_ ## NAME ## _OFFSET))
