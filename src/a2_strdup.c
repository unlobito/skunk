#include <pebble.h>
#include "a2_strdup.h"

char *a2_strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *dup = malloc(len);
    strcpy(dup, str);
    return dup;
}
