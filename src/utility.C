// THOR - The God of Thunder
// See LICENSE for details

#include "utility.h"

#include <stdlib.h>
#include <time.h>

void delay(int milliseconds)
{
  long pause;
  clock_t now, then;

  pause = milliseconds * (CLOCKS_PER_SEC / 1000);
  now = then = clock();
  while ((now - then) < pause)
    now = clock();
}

char* end;

int8_t parse_decimal_int8_t(char* str) {
    return strtol(str, &end, 10);
}

int16_t parse_decimal_int16_t(char* str) {
    return strtol(str, &end, 10);
}

int32_t parse_decimal_int32_t(char* str) {
    return strtol(str, &end, 10);
}
