#include "comma_background_layer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "comma_glyphs.h"
#include "comma_layout.h"
#include "comma_palette.h"

#define COMMA_BG_CELL_COUNT (COMMA_GRID_COLS * COMMA_GRID_ROWS)
#define COMMA_BG_FRAME_MS 16 /* target ~60fps */
#define COMMA_BG_CELL_ANIM_MS 1040
#define COMMA_BG_CELL_STAGGER_MIN_MS 0
#define COMMA_BG_CELL_STAGGER_MAX_MS 336
#define COMMA_BG_ACTIVATION_DURATION_MS 416
#define COMMA_BG_ACTIVE_PERCENT 18
#define COMMA_BG_ACTIVE_DIGIT_PERCENT 100
#define COMMA_BG_INTRO_DELAY_MS 120

typedef struct {
  int32_t elapsed_ms;
  int32_t start_delay_ms;
  bool complete;
  bool active;
  bool is_digit;
} CommaBackgroundCellState;

typedef struct {
  CommaBackgroundCellState cells[COMMA_BG_CELL_COUNT];
  bool animation_complete;
  bool intro_complete;
  int32_t intro_elapsed_ms;
  int32_t activation_window_ms;
  float activation_ratio;
} CommaBackgroundLayerState;

struct CommaBackgroundLayer {
  Layer *layer;
  CommaBackgroundLayerState *state;
  AppTimer *timer;
};

static inline CommaBackgroundLayerState *prv_get_state(CommaBackgroundLayer *layer) {
  return (layer && layer->layer) ? layer->state : NULL;
}

static bool s_digit_cell_mask_initialized;
static bool s_digit_cell_mask[COMMA_BG_CELL_COUNT];

static inline int prv_cell_index(int col, int row) {
  return (row * COMMA_GRID_COLS) + col;
}

static float prv_ease(float t) {
  if (t < 0.0f) {
    t = 0.0f;
  } else if (t > 1.0f) {
    t = 1.0f;
  }
  const float inv = 1.0f - t;
  return 1.0f - (inv * inv * inv);
}

static void prv_seed_random(void) {
  static bool s_seeded = false;
  if (s_seeded) {
    return;
  }
  s_seeded = true;
  srand((unsigned int)time(NULL));
}

static int32_t prv_random_range(int32_t min_inclusive, int32_t max_inclusive) {
  if (max_inclusive <= min_inclusive) {
    return min_inclusive;
  }
  const int32_t span = max_inclusive - min_inclusive + 1;
  return min_inclusive + (rand() % span);
}

static float prv_digit_center_col(void) {
  const float total_width =
      (COMMA_DIGIT_WIDTH * COMMA_DIGIT_COUNT) + COMMA_DIGIT_COLON_WIDTH +
      (COMMA_DIGIT_GAP * 4);
  return COMMA_DIGIT_START_COL + (total_width / 2.0f);
}

static float prv_digit_center_row(void) {
  return COMMA_DIGIT_START_ROW + (COMMA_DIGIT_HEIGHT / 2.0f);
}

static float prv_cell_bias(int cell_col, int cell_row) {
  const float col_center = prv_digit_center_col();
  const float row_center = prv_digit_center_row();

  const float col_half_span =
      ((COMMA_DIGIT_WIDTH * COMMA_DIGIT_COUNT) + COMMA_DIGIT_COLON_WIDTH) /
      2.0f;
  const float row_half_span = (COMMA_DIGIT_HEIGHT / 2.0f);

  const float col_dist = fabsf(((float)cell_col) - col_center) /
                         (col_half_span + 1.0f);
  const float row_dist =
      fabsf(((float)cell_row) - row_center) / (row_half_span + 1.0f);

  float bias = 0.0f;
  if (col_dist < 1.0f) {
    bias += (1.0f - col_dist);
  }
  if (row_dist < 1.0f) {
    bias += (1.0f - row_dist) * 0.8f;
  }
  bias /= 1.8f; /* normalize back into 0..~1 */
  if (bias < 0.0f) {
    bias = 0.0f;
  } else if (bias > 1.0f) {
    bias = 1.0f;
  }
  return bias;
}

static bool prv_should_activate_cell(int cell_col, int cell_row) {
  const int idx = prv_cell_index(cell_col, cell_row);
  const bool in_digit = s_digit_cell_mask_initialized && s_digit_cell_mask[idx];
  if (in_digit) {
    return true;
  }
  int percent =
      COMMA_BG_ACTIVE_PERCENT +
      (int)(prv_cell_bias(cell_col, cell_row) * 32.0f);
  if (percent > 100) {
    percent = 100;
  }
  const int roll = rand() % 100;
  return roll < percent;
}

static void prv_reset_cell(CommaBackgroundCellState *cell) {
  if (!cell) {
    return;
  }
  if (!cell->active) {
    cell->elapsed_ms = 0;
    cell->start_delay_ms = 0;
    cell->complete = true;
    return;
  }
  cell->elapsed_ms = 0;
  cell->complete = false;
  cell->start_delay_ms =
      prv_random_range(COMMA_BG_CELL_STAGGER_MIN_MS, COMMA_BG_CELL_STAGGER_MAX_MS);
}

static void prv_mark_glyph_cells(int base_col, const CommaGlyph *glyph) {
  if (!glyph) {
    return;
  }
  const int base_row = COMMA_DIGIT_START_ROW;
  for (int row = 0; row < COMMA_DIGIT_HEIGHT; ++row) {
    const uint8_t mask = glyph->rows[row];
    if (!mask) {
      continue;
    }
    for (int col = 0; col < glyph->width; ++col) {
      if (mask & (1 << (glyph->width - 1 - col))) {
        const int cell_col = base_col + col;
        const int cell_row = base_row + row;
        if (cell_col >= 0 && cell_col < COMMA_GRID_COLS &&
            cell_row >= 0 && cell_row < COMMA_GRID_ROWS) {
          s_digit_cell_mask[prv_cell_index(cell_col, cell_row)] = true;
        }
      }
    }
  }
}

static void prv_init_digit_cell_mask(void) {
  if (s_digit_cell_mask_initialized) {
    return;
  }
  memset(s_digit_cell_mask, 0, sizeof(s_digit_cell_mask));

  int col = COMMA_DIGIT_START_COL;
  for (int slot = 0; slot < 2; ++slot) {
    for (int glyph = COMMA_GLYPH_ZERO; glyph <= COMMA_GLYPH_NINE; ++glyph) {
      prv_mark_glyph_cells(col, &COMMA_GLYPHS[glyph]);
    }
    col += COMMA_DIGIT_WIDTH + COMMA_DIGIT_GAP;
  }

  prv_mark_glyph_cells(col, &COMMA_GLYPHS[COMMA_GLYPH_COLON]);
  col += COMMA_DIGIT_COLON_WIDTH + COMMA_DIGIT_GAP;

  for (int slot = 0; slot < 2; ++slot) {
    for (int glyph = COMMA_GLYPH_ZERO; glyph <= COMMA_GLYPH_NINE; ++glyph) {
      prv_mark_glyph_cells(col, &COMMA_GLYPHS[glyph]);
    }
    col += COMMA_DIGIT_WIDTH + COMMA_DIGIT_GAP;
  }

  s_digit_cell_mask_initialized = true;
}

static void prv_init_cells(CommaBackgroundLayerState *state) {
  if (!state) {
    return;
  }
  state->animation_complete = false;
  state->intro_complete = false;
  state->intro_elapsed_ms = 0;
  state->activation_window_ms = COMMA_BG_CELL_STAGGER_MIN_MS;
  state->activation_ratio = 0.0f;
  for (int idx = 0; idx < COMMA_BG_CELL_COUNT; ++idx) {
    const int cell_row = idx / COMMA_GRID_COLS;
    const int cell_col = idx % COMMA_GRID_COLS;
    CommaBackgroundCellState *cell = &state->cells[idx];
    cell->active = prv_should_activate_cell(cell_col, cell_row);
    cell->is_digit = s_digit_cell_mask[idx];
    prv_reset_cell(cell);
  }
}

static void prv_draw_row_span(GContext *ctx, const GPoint origin, int row,
                              int col_start, int col_end) {
  if (col_end < col_start) {
    return;
  }
  for (int col = col_start; col <= col_end; ++col) {
    graphics_draw_pixel(ctx, GPoint(origin.x + col, origin.y + row));
  }
}

static void prv_draw_background_shape(GContext *ctx, const GPoint origin,
                                      int size_level) {
  switch (size_level) {
    case 2:
      for (int row = 1; row <= 4; ++row) {
        prv_draw_row_span(ctx, origin, row, 1, 4);
      }
      break;
    case 1:
      prv_draw_row_span(ctx, origin, 1, 2, 3);
      for (int row = 2; row <= 3; ++row) {
        prv_draw_row_span(ctx, origin, row, 1, 4);
      }
      prv_draw_row_span(ctx, origin, 4, 2, 3);
      break;
    case 0:
      for (int row = 2; row <= 3; ++row) {
        prv_draw_row_span(ctx, origin, row, 2, 3);
      }
      break;
    default:
      break;
  }
}

static void prv_draw_background_cell(GContext *ctx, int cell_col, int cell_row,
                                     int size_level) {
  if (size_level < 0) {
    return;
  }
  const GRect frame = comma_cell_frame(cell_col, cell_row);
  prv_draw_background_shape(ctx, frame.origin, size_level);
}

static bool prv_cell_progress_value(const CommaBackgroundLayerState *state,
                                    const CommaBackgroundCellState *cell,
                                    float *progress_out) {
  if (!state || !cell || !progress_out) {
    return false;
  }

  if (!state->intro_complete ||
      cell->start_delay_ms > state->activation_window_ms) {
    *progress_out = 0.0f;
    return false;
  }

  if (!cell->active) {
    *progress_out = 0.0f;
    return false;
  }

  int32_t local = cell->elapsed_ms - cell->start_delay_ms;
  if (local <= 0 && !cell->complete) {
    *progress_out = 0.0f;
    return false;
  }

  if (local < 0) {
    local = 0;
  }
  const int32_t total = COMMA_BG_CELL_ANIM_MS;
  if (local > total) {
    local = total;
  }
  float progress = (float)local / (float)total;
  *progress_out = prv_ease(progress);
  return true;
}

static int prv_shape_level_for_progress(float progress) {
  if (progress <= 0.0f) {
    return -1;
  }
  if (progress < 0.28f) {
    return 2;
  }
  if (progress < 0.6f) {
    return 1;
  }
  if (progress < 0.92f) {
    return 0;
  }
  return -1;
}

static GColor prv_color_from_stage(int stage) {
#if defined(PBL_COLOR)
  switch (stage) {
    case 0:
      return GColorFromRGB(0x55, 0x55, 0x55);
    case 1:
      return GColorFromRGB(0xAA, 0xAA, 0xAA);
    default:
      return GColorFromRGB(0xFF, 0xFF, 0xFF);
  }
#else
  (void)stage;
  return GColorWhite;
#endif
}

static GColor prv_color_for_progress(float progress, bool is_digit) {
  if (progress < 0.0f) {
    progress = 0.0f;
  } else if (progress > 1.0f) {
    progress = 1.0f;
  }
  if (is_digit) {
    if (progress < (1.0f / 3.0f)) {
      return prv_color_from_stage(0);
    } else if (progress < (2.0f / 3.0f)) {
      return prv_color_from_stage(1);
    }
    return prv_color_from_stage(2);
  }

  if (progress < 0.5f) {
    const float phase = progress / 0.5f;
    if (phase < (1.0f / 3.0f)) {
      return prv_color_from_stage(0);
    } else if (phase < (2.0f / 3.0f)) {
      return prv_color_from_stage(1);
    }
    return prv_color_from_stage(2);
  }
  const float phase = (progress - 0.5f) / 0.5f;
  if (phase < (1.0f / 3.0f)) {
    return prv_color_from_stage(2);
  } else if (phase < (2.0f / 3.0f)) {
    return prv_color_from_stage(1);
  }
  return prv_color_from_stage(0);
}

static bool prv_step_animation(CommaBackgroundLayer *layer) {
  CommaBackgroundLayerState *state = prv_get_state(layer);
  if (!state) {
    return true;
  }

  if (!state->intro_complete) {
    state->intro_elapsed_ms += COMMA_BG_FRAME_MS;
    if (state->intro_elapsed_ms >= COMMA_BG_INTRO_DELAY_MS) {
      state->intro_complete = true;
      state->activation_window_ms = COMMA_BG_CELL_STAGGER_MIN_MS;
    }
    return false;
  }

  if (state->activation_ratio < 1.0f &&
      COMMA_BG_ACTIVATION_DURATION_MS > 0) {
    state->activation_ratio +=
        (float)COMMA_BG_FRAME_MS / (float)COMMA_BG_ACTIVATION_DURATION_MS;
    if (state->activation_ratio > 1.0f) {
      state->activation_ratio = 1.0f;
    }
    const float eased =
        prv_ease(state->activation_ratio);
    const int span =
        COMMA_BG_CELL_STAGGER_MAX_MS - COMMA_BG_CELL_STAGGER_MIN_MS;
    state->activation_window_ms =
        COMMA_BG_CELL_STAGGER_MIN_MS + (int)((float)span * eased);
  }

  bool all_complete = true;
  for (int idx = 0; idx < COMMA_BG_CELL_COUNT; ++idx) {
    CommaBackgroundCellState *cell = &state->cells[idx];
    if (!cell->active) {
      continue;
    }
    if (cell->start_delay_ms > state->activation_window_ms) {
      all_complete = false;
      continue;
    }
    const int32_t max_elapsed =
        cell->start_delay_ms + COMMA_BG_CELL_ANIM_MS;
    if (cell->complete && cell->elapsed_ms >= max_elapsed) {
      continue;
    }

    all_complete = false;
    cell->elapsed_ms += COMMA_BG_FRAME_MS;
    if (cell->elapsed_ms >= max_elapsed) {
      cell->elapsed_ms = max_elapsed;
      cell->complete = true;
    }
  }

  if (all_complete) {
    state->animation_complete = true;
  }
  return state->animation_complete;
}

static void prv_timer_proc(void *ctx);

static void prv_schedule_timer(CommaBackgroundLayer *layer) {
  if (!layer) {
    return;
  }
  CommaBackgroundLayerState *state = prv_get_state(layer);
  if (state && state->animation_complete) {
    layer->timer = NULL;
    return;
  }
  layer->timer = app_timer_register(COMMA_BG_FRAME_MS, prv_timer_proc, layer);
}

static void prv_timer_proc(void *ctx) {
  CommaBackgroundLayer *layer = ctx;
  if (!layer || !layer->layer) {
    return;
  }
  const bool done = prv_step_animation(layer);
  layer_mark_dirty(layer->layer);
  if (!done) {
    prv_schedule_timer(layer);
  } else {
    layer->timer = NULL;
  }
}

static void prv_start_animation(CommaBackgroundLayer *layer) {
  if (!layer) {
    return;
  }
  if (layer->timer) {
    app_timer_cancel(layer->timer);
    layer->timer = NULL;
  }
  CommaBackgroundLayerState *state = prv_get_state(layer);
  if (state) {
    state->animation_complete = false;
    state->intro_complete = false;
    state->intro_elapsed_ms = 0;
    state->activation_window_ms = COMMA_BG_CELL_STAGGER_MIN_MS;
    state->activation_ratio = 0.0f;
    for (int idx = 0; idx < COMMA_BG_CELL_COUNT; ++idx) {
      const int cell_row = idx / COMMA_GRID_COLS;
      const int cell_col = idx % COMMA_GRID_COLS;
      CommaBackgroundCellState *cell = &state->cells[idx];
      cell->active = prv_should_activate_cell(cell_col, cell_row);
      cell->is_digit = s_digit_cell_mask[idx];
      prv_reset_cell(cell);
    }
  }
  prv_schedule_timer(layer);
}

static void prv_stop_animation(CommaBackgroundLayer *layer) {
  if (!layer || !layer->timer) {
    return;
  }
  app_timer_cancel(layer->timer);
  layer->timer = NULL;
}

static void prv_background_update_proc(Layer *layer_ref, GContext *ctx) {
  CommaBackgroundLayerState *state = layer_get_data(layer_ref);
  if (!state) {
    return;
  }

  const GRect bounds = layer_get_bounds(layer_ref);
  graphics_context_set_fill_color(ctx, comma_palette_background_fill());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, comma_palette_background_stroke());
  for (int row = 0; row < COMMA_GRID_ROWS; ++row) {
    for (int col = 0; col < COMMA_GRID_COLS; ++col) {
      prv_draw_background_cell(ctx, col, row, 0);
    }
  }

  int cell_index = 0;
  for (int row = 0; row < COMMA_GRID_ROWS; ++row) {
    for (int col = 0; col < COMMA_GRID_COLS; ++col, ++cell_index) {
      CommaBackgroundCellState *cell = &state->cells[cell_index];
      float progress = 0.0f;
      if (!prv_cell_progress_value(state, cell, &progress)) {
        continue;
      }
      const int size_level = prv_shape_level_for_progress(progress);
      if (size_level < 0) {
        continue;
      }
      const GColor color = prv_color_for_progress(progress, cell->is_digit);
      graphics_context_set_stroke_color(ctx, color);
      prv_draw_background_cell(ctx, col, row, size_level);
    }
  }
}

CommaBackgroundLayer *comma_background_layer_create(GRect frame) {
  prv_seed_random();
  prv_init_digit_cell_mask();

  CommaBackgroundLayer *layer = calloc(1, sizeof(*layer));
  if (!layer) {
    return NULL;
  }

  layer->layer =
      layer_create_with_data(frame, sizeof(CommaBackgroundLayerState));
  if (!layer->layer) {
    free(layer);
    return NULL;
  }

  layer->state = layer_get_data(layer->layer);
  prv_init_cells(layer->state);

  layer_set_update_proc(layer->layer, prv_background_update_proc);
  prv_start_animation(layer);
  return layer;
}

void comma_background_layer_destroy(CommaBackgroundLayer *layer) {
  if (!layer) {
    return;
  }

  prv_stop_animation(layer);

  if (layer->layer) {
    layer_destroy(layer->layer);
    layer->layer = NULL;
    layer->state = NULL;
  }
  free(layer);
}

Layer *comma_background_layer_get_layer(CommaBackgroundLayer *layer) {
  return layer ? layer->layer : NULL;
}

void comma_background_layer_mark_dirty(CommaBackgroundLayer *layer) {
  if (layer && layer->layer) {
    layer_mark_dirty(layer->layer);
  }
}

bool comma_background_layer_cell_progress(CommaBackgroundLayer *layer,
                                           int cell_col,
                                           int cell_row,
                                           float *progress_out) {
  CommaBackgroundLayerState *state = prv_get_state(layer);
  if (!state || !progress_out) {
    return false;
  }
  if (cell_col < 0 || cell_col >= COMMA_GRID_COLS || cell_row < 0 ||
      cell_row >= COMMA_GRID_ROWS) {
    *progress_out = 0.0f;
    return false;
  }
  const int idx = prv_cell_index(cell_col, cell_row);
  const CommaBackgroundCellState *cell = &state->cells[idx];
  return prv_cell_progress_value(state, cell, progress_out);
}
