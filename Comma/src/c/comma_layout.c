#include "comma_layout.h"

#include <pebble.h>

static CommaLayout s_layout = {
  .grid_cols = 24,
  .grid_rows = 28,
  .digit_start_col = 1,
  .digit_start_row = 10,
  .offset_x = 0,
  .offset_y = 0,
};

static int prv_clamp(int value, int min_value, int max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

void comma_layout_configure(GSize bounds) {
  if (bounds.w <= 0 || bounds.h <= 0) {
    return;
  }

  const int min_cols = COMMA_DIGIT_SPAN_COLS;
  const int min_rows = COMMA_DIGIT_HEIGHT;

  int cols = bounds.w / COMMA_CELL_SIZE;
  int rows = bounds.h / COMMA_CELL_SIZE;

  cols = prv_clamp(cols, min_cols, COMMA_BG_MAX_COLS);
  rows = prv_clamp(rows, min_rows, COMMA_BG_MAX_ROWS);

  const int remaining_cols = cols - COMMA_DIGIT_SPAN_COLS;
  const int remaining_rows = rows - COMMA_DIGIT_HEIGHT;

  s_layout.grid_cols = cols;
  s_layout.grid_rows = rows;
  s_layout.digit_start_col = (remaining_cols > 0) ? (remaining_cols / 2) : 0;
  s_layout.digit_start_row = (remaining_rows > 0) ? (remaining_rows / 2) : 0;

  const int used_width = cols * COMMA_CELL_SIZE;
  const int used_height = rows * COMMA_CELL_SIZE;
  s_layout.offset_x = (bounds.w - used_width) / 2;
  s_layout.offset_y = (bounds.h - used_height) / 2;
  if (s_layout.offset_x < 0) {
    s_layout.offset_x = 0;
  }
  if (s_layout.offset_y < 0) {
    s_layout.offset_y = 0;
  }
}

const CommaLayout *comma_layout_get(void) {
  return &s_layout;
}
