#pragma once

#include <pebble.h>

#define GENERAL_MAGIC_BG_FRAME_MS 16 /* target ~60fps */
#define GENERAL_MAGIC_BG_BASE_CELL_ANIM_MS 1300
#define GENERAL_MAGIC_BG_BASE_CELL_STAGGER_MIN_MS 0
#define GENERAL_MAGIC_BG_BASE_CELL_STAGGER_MAX_MS 420
#define GENERAL_MAGIC_BG_BASE_ACTIVATION_DURATION_MS 520
#define GENERAL_MAGIC_BG_ACTIVE_PERCENT 18
#define GENERAL_MAGIC_BG_ACTIVE_DIGIT_PERCENT 100
#define GENERAL_MAGIC_BG_BASE_INTRO_DELAY_MS 120

#define GENERAL_MAGIC_REFERENCE_SCREEN_WIDTH 200
#define GENERAL_MAGIC_REFERENCE_SCREEN_HEIGHT 228
#define GENERAL_MAGIC_REFERENCE_CELL_SIZE 8
#define GENERAL_MAGIC_REFERENCE_COLS (GENERAL_MAGIC_REFERENCE_SCREEN_WIDTH / GENERAL_MAGIC_REFERENCE_CELL_SIZE)
#define GENERAL_MAGIC_REFERENCE_ROWS (GENERAL_MAGIC_REFERENCE_SCREEN_HEIGHT / GENERAL_MAGIC_REFERENCE_CELL_SIZE)

typedef struct {
  int32_t intro_delay_ms;
  int32_t cell_anim_ms;
  int32_t activation_duration_ms;
} GeneralMagicBackgroundTiming;

typedef struct GeneralMagicBackgroundLayer GeneralMagicBackgroundLayer;

GeneralMagicBackgroundLayer *general_magic_background_layer_create(GRect frame);
void general_magic_background_layer_destroy(GeneralMagicBackgroundLayer *layer);
Layer *general_magic_background_layer_get_layer(GeneralMagicBackgroundLayer *layer);
void general_magic_background_layer_mark_dirty(GeneralMagicBackgroundLayer *layer);
bool general_magic_background_layer_cell_progress(GeneralMagicBackgroundLayer *layer,
                                                  int cell_col,
                                                  int cell_row,
                                                  float *progress_out);
void general_magic_background_layer_set_animated(GeneralMagicBackgroundLayer *layer,
                                                 bool animated);
bool general_magic_background_layer_get_timing(GeneralMagicBackgroundLayer *layer,
                                               GeneralMagicBackgroundTiming *timing_out);
