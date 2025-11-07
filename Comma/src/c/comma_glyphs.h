#pragma once

#include <pebble.h>

#include "comma_layout.h"

typedef struct {
  uint8_t width;
  uint8_t rows[COMMA_DIGIT_HEIGHT];
  uint8_t pins[COMMA_DIGIT_HEIGHT];
} CommaGlyph;

enum {
  COMMA_GLYPH_ZERO = 0,
  COMMA_GLYPH_ONE,
  COMMA_GLYPH_TWO,
  COMMA_GLYPH_THREE,
  COMMA_GLYPH_FOUR,
  COMMA_GLYPH_FIVE,
  COMMA_GLYPH_SIX,
  COMMA_GLYPH_SEVEN,
  COMMA_GLYPH_EIGHT,
  COMMA_GLYPH_NINE,
  COMMA_GLYPH_COLON,
  COMMA_GLYPH_COUNT
};

extern const CommaGlyph COMMA_GLYPHS[COMMA_GLYPH_COUNT];
