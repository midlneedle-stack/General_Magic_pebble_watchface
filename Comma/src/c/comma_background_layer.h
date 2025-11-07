#pragma once

#include <pebble.h>

typedef struct CommaBackgroundLayer CommaBackgroundLayer;

CommaBackgroundLayer *comma_background_layer_create(GRect frame);
void comma_background_layer_destroy(CommaBackgroundLayer *layer);
Layer *comma_background_layer_get_layer(CommaBackgroundLayer *layer);
void comma_background_layer_mark_dirty(CommaBackgroundLayer *layer);
bool comma_background_layer_cell_progress(CommaBackgroundLayer *layer,
                                           int cell_col,
                                           int cell_row,
                                           float *progress_out);
