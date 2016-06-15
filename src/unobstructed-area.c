#include <pebble.h>

static Window *s_main_window;
static Layer *s_window_layer;
static TextLayer *s_text_layer;

// Is the screen obstructed?
static bool s_screen_is_obstructed;

// Event fires once, before the obstruction appears or disappears
static void prv_unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {
  if (s_screen_is_obstructed) {
    // Obstruction is about disappear
  } else {
    // Obstruction is about to appear
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG,
    "Available screen area: width: %d, height: %d",
    final_unobstructed_screen_area.size.w,
    final_unobstructed_screen_area.size.h);
}

// Event fires once, after obstruction appears or disappears
static void prv_unobstructed_did_change(void *context) {
  // Keep track if the screen is obstructed or not
  s_screen_is_obstructed = !s_screen_is_obstructed;

  if(s_screen_is_obstructed) {
    text_layer_set_text(s_text_layer, "Obstructed!");
  } else {
    text_layer_set_text(s_text_layer, "Unobstructed!");
  }
}

// Event fires frequently, while obstruction is appearing or disappearing
static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Progress %d", (int)progress);

  text_layer_set_text(s_text_layer, "Changing!");

  // Current unobstructed window size
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);

  // Move the text layer
  GRect text_frame = layer_get_frame(text_layer_get_layer(s_text_layer));
  text_frame.origin.y = (bounds.size.h/2)-10;
  layer_set_frame(text_layer_get_layer(s_text_layer), text_frame);
}

static void prv_main_window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  GRect fullscreen = layer_get_bounds(s_window_layer);
  GRect visible_bounds = layer_get_unobstructed_bounds(s_window_layer);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Full %d", (int)fullscreen.size.h);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Visible %d", (int)visible_bounds.size.h);

  // Determine if the screen is obstructed when the app starts
  s_screen_is_obstructed = !grect_equal(&fullscreen, &visible_bounds);

  // Create a text layer
  s_text_layer = text_layer_create(GRect(0, (visible_bounds.size.h/2)-10,
    visible_bounds.size.w, 20));
  text_layer_set_background_color(s_text_layer, GColorWhite);
  text_layer_set_text_color(s_text_layer, GColorBlack);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  if(s_screen_is_obstructed) {
    text_layer_set_text(s_text_layer, "Obstructed!");
  } else {
    text_layer_set_text(s_text_layer, "Unobstructed!");
  }
  layer_add_child(s_window_layer, text_layer_get_layer(s_text_layer));

  // Subscribe to the unobstructed area events
   UnobstructedAreaHandlers handlers = {
    .will_change = prv_unobstructed_will_change,
    .change = prv_unobstructed_change,
    .did_change = prv_unobstructed_did_change
  };
}

static void prv_main_window_unload(Window *window) {
  // Unsubscribe from the unobstructed area service
  unobstructed_area_service_unsubscribe();

  text_layer_destroy(s_text_layer);
}

static void prv_init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = prv_main_window_load,
    .unload = prv_main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void prv_deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
