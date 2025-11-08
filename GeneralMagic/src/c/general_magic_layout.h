#pragma once

#include <pebble.h>

#if defined(PBL_PLATFORM_EMERY)
#define GENERAL_MAGIC_CELL_SIZE 8
#else
#define GENERAL_MAGIC_CELL_SIZE 6
#endif

enum {
  GENERAL_MAGIC_DIGIT_WIDTH = 4,
  GENERAL_MAGIC_DIGIT_HEIGHT = 9,
  GENERAL_MAGIC_DIGIT_COLON_WIDTH = 2,
  GENERAL_MAGIC_DIGIT_COUNT = 4,
  GENERAL_MAGIC_DIGIT_GAP = 1,
};

#define GENERAL_MAGIC_TOTAL_GLYPHS (GENERAL_MAGIC_DIGIT_COUNT + 1)
#define GENERAL_MAGIC_DIGIT_SPAN_COLS \
  ((GENERAL_MAGIC_DIGIT_WIDTH * GENERAL_MAGIC_DIGIT_COUNT) + GENERAL_MAGIC_DIGIT_COLON_WIDTH + \
   (GENERAL_MAGIC_DIGIT_GAP * (GENERAL_MAGIC_TOTAL_GLYPHS - 1)))

#define GENERAL_MAGIC_BG_MAX_COLS 35
#define GENERAL_MAGIC_BG_MAX_ROWS 40
#define GENERAL_MAGIC_BG_CELL_CAPACITY (GENERAL_MAGIC_BG_MAX_COLS * GENERAL_MAGIC_BG_MAX_ROWS)

typedef struct {
  int grid_cols;
  int grid_rows;
  int digit_start_col;
  int digit_start_row;
  int offset_x;
  int offset_y;
} GeneralMagicLayout;

void general_magic_layout_configure(GSize bounds);
const GeneralMagicLayout *general_magic_layout_get(void);

static inline GPoint general_magic_cell_origin(int cell_col, int cell_row) {
  const GeneralMagicLayout *layout = general_magic_layout_get();
  return GPoint(layout->offset_x + (cell_col * GENERAL_MAGIC_CELL_SIZE),
                layout->offset_y + (cell_row * GENERAL_MAGIC_CELL_SIZE));
}

static inline GRect general_magic_cell_frame(int cell_col, int cell_row) {
  const GeneralMagicLayout *layout = general_magic_layout_get();
  return GRect(layout->offset_x + (cell_col * GENERAL_MAGIC_CELL_SIZE),
               layout->offset_y + (cell_row * GENERAL_MAGIC_CELL_SIZE),
               GENERAL_MAGIC_CELL_SIZE, GENERAL_MAGIC_CELL_SIZE);
}
