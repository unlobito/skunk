#include "tobinstr.h"

// http://stackoverflow.com/a/7911672/108511
void tobinstr(int value, int bitsCount, char* output)
{
    int i;
    output[bitsCount] = '\0';
    for (i = bitsCount - 1; i >= 0; --i, value >>= 1)
    {
        output[i] = (value & 1) + '0';
    }
}
