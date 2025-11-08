#include <pebble.h>

#include "general_magic_background_layer.h"
#include "general_magic_digit_layer.h"
#include "general_magic_layout.h"
#include "general_magic_palette.h"

static Window *s_main_window;
static GeneralMagicBackgroundLayer *s_background_layer;
static GeneralMagicDigitLayer *s_digit_layer;

static const uint32_t s_intro_vibe_segments[] = {
    /* single tap – light intro */
    24, 160,
    /* two taps – beginning to swell */
    26, 120, 29, 150,
    /* three taps – mid lift */
    30, 95, 28, 95, 26, 150,
    /* four taps – peak intensity */
    34, 75, 32, 75, 30, 75, 28, 170,
    /* taper back down */
    26, 140, 25, 210,
    22, 250, 20, 340,
};

static void prv_play_intro_vibe(void) {
  if (quiet_time_is_active()) {
    return;
  }
  const VibePattern pattern = {
      .durations = s_intro_vibe_segments,
      .num_segments = ARRAY_LENGTH(s_intro_vibe_segments),
  };
  vibes_cancel();
  vibes_enqueue_custom_pattern(pattern);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  (void)units_changed;
  general_magic_digit_layer_set_time(s_digit_layer, tick_time);
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(root);

  general_magic_layout_configure(bounds.size);
  s_background_layer = general_magic_background_layer_create(bounds);
  if (s_background_layer) {
    layer_add_child(root, general_magic_background_layer_get_layer(s_background_layer));
  }

  s_digit_layer = general_magic_digit_layer_create(bounds);
  if (s_digit_layer) {
    layer_add_child(root, general_magic_digit_layer_get_layer(s_digit_layer));
    general_magic_digit_layer_bind_background(s_digit_layer, s_background_layer);
    general_magic_digit_layer_refresh_time(s_digit_layer);
  }
}

static void prv_window_unload(Window *window) {
  (void)window;

  general_magic_digit_layer_destroy(s_digit_layer);
  s_digit_layer = NULL;

  general_magic_background_layer_destroy(s_background_layer);
  s_background_layer = NULL;
}

static void prv_window_appear(Window *window) {
  (void)window;
  if (s_digit_layer) {
    general_magic_digit_layer_start_diag_flip(s_digit_layer);
  }
  prv_play_intro_vibe();
}

static void prv_init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, general_magic_palette_window_background());
  window_set_window_handlers(s_main_window, (WindowHandlers){
                                            .load = prv_window_load,
                                            .appear = prv_window_appear,
                                            .unload = prv_window_unload,
                                          });

  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
  s_main_window = NULL;
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
