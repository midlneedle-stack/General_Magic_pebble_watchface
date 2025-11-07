#pragma once

#include <pebble.h>

enum {
  COMMA_GRID_COLS = 24,
  COMMA_GRID_ROWS = 28,
  COMMA_CELL_SIZE = 6,
  COMMA_DIGIT_WIDTH = 4,
  COMMA_DIGIT_HEIGHT = 9,
  COMMA_DIGIT_COLON_WIDTH = 2,
  COMMA_DIGIT_COUNT = 4,
  COMMA_DIGIT_GAP = 1,
  COMMA_DIGIT_START_COL = 1,
  COMMA_DIGIT_START_ROW = 10,
};

static inline GPoint comma_cell_origin(int cell_col, int cell_row) {
  return GPoint(cell_col * COMMA_CELL_SIZE, cell_row * COMMA_CELL_SIZE);
}

static inline GRect comma_cell_frame(int cell_col, int cell_row) {
  return GRect(cell_col * COMMA_CELL_SIZE, cell_row * COMMA_CELL_SIZE,
               COMMA_CELL_SIZE, COMMA_CELL_SIZE);
}
