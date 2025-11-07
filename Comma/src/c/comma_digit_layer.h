#pragma once

#include <pebble.h>

typedef struct CommaDigitLayer CommaDigitLayer;
typedef struct CommaBackgroundLayer CommaBackgroundLayer;

CommaDigitLayer *comma_digit_layer_create(GRect frame);
void comma_digit_layer_destroy(CommaDigitLayer *layer);
Layer *comma_digit_layer_get_layer(CommaDigitLayer *layer);
void comma_digit_layer_bind_background(CommaDigitLayer *layer,
                                        CommaBackgroundLayer *background);
void comma_digit_layer_set_time(CommaDigitLayer *layer, const struct tm *time);
void comma_digit_layer_refresh_time(CommaDigitLayer *layer);
void comma_digit_layer_force_redraw(CommaDigitLayer *layer);
/** Trigger the artifact-style reveal animation for the digits. */
void comma_digit_layer_start_diag_flip(CommaDigitLayer *layer);
