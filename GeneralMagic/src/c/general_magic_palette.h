#pragma once

#include <pebble.h>

static inline GColor general_magic_palette_background_fill(void) {
  return GColorBlack;
}

static inline GColor general_magic_palette_background_stroke(void) {
  return PBL_IF_COLOR_ELSE(GColorFromRGB(0x55, 0x55, 0x55), GColorBlack);
}

static inline GColor general_magic_palette_digit_fill(void) {
  return general_magic_palette_background_fill();
}

static inline GColor general_magic_palette_digit_stroke(void) {
  return GColorWhite;
}

static inline GColor general_magic_palette_window_background(void) {
  return PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite);
}
