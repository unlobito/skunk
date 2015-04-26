#pragma once

#define PEBBLE_HEIGHT 168
#define PEBBLE_WIDTH 144
#define STATUS_HEIGHT 16

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
