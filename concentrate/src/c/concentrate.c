#include <pebble.h>
#include "ticks.h"

static Window *s_main_window;
static Layer *s_main_layer;
static TextLayer *s_step_layer;
static GFont s_dos_font;

static GPath *s_tick_paths[12];

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
}

static void health_handler(HealthEventType event, void *context) {
  static char s_step_buffer[8];
  if (event == HealthEventMovementUpdate) {
    snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    text_layer_set_text(s_step_layer, s_step_buffer);
  }
}

static void main_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  GRect layer_bounds = layer_get_bounds(layer);
  GPoint layer_center = grect_center_point(&layer_bounds);
  uint16_t radius = layer_bounds.size.w/2;
  
  //Background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  
  //Hands
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_radial(ctx, GRect(24, 24, layer_bounds.size.w-48, layer_bounds.size.h-48),
                       GOvalScaleModeFitCircle, 24, DEG_TO_TRIGANGLE(0),
                       (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10))) / (12 * 6));
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_radial(ctx, layer_bounds,
                       GOvalScaleModeFitCircle, 24, DEG_TO_TRIGANGLE(0),
                       (TRIG_MAX_ANGLE*tick_time->tm_min/60));
  
  //Ticks
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(layer_center.x-4, -2, 8, 26), 4, (GCornerBottomLeft|GCornerBottomRight));
  //for (int i=0; i<12; ++i) {
  //  if (i == 0) {
  //    gpath_rotate_to(s_tick_paths[i], (TRIG_MAX_ANGLE*i/12));
  //  } else {
  //    gpath_rotate_to(s_tick_paths[i], (TRIG_MAX_ANGLE*i/12));
  //    gpath_draw_filled(ctx, s_tick_paths[i]);
  //  }
  //}
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  
  s_dos_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DOSOS_42));
  
  s_main_layer = layer_create(bounds);
  layer_set_update_proc(s_main_layer, main_update_proc);
  layer_add_child(window_get_root_layer(window), s_main_layer);
  
  //Stepcount text
  s_step_layer = text_layer_create(GRect(0, bounds.size.h/2-30, bounds.size.w, 50));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_text_color(s_step_layer, GColorWhite);
  text_layer_set_font(s_step_layer, s_dos_font);
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
  
  for (int i=0; i<12; ++i) {
    s_tick_paths[i] = gpath_create(&TICKS);
    gpath_move_to(s_tick_paths[i],
                  GPoint(((sin_lookup(TRIG_MAX_ANGLE*i/12)*(bounds.size.w/2)/TRIG_MAX_RATIO)+center.x),
                         (-cos_lookup(TRIG_MAX_ANGLE*i/12)*(bounds.size.h/2)/TRIG_MAX_RATIO)+center.y));
  }
  
  //Health subscription
  if (health_service_events_subscribe(health_handler, NULL)) {
    health_handler(HealthEventMovementUpdate, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    text_layer_set_text(s_step_layer, "Error!");
  }
}

static void main_window_unload(Window *window) {
  layer_destroy(s_main_layer);
  text_layer_destroy(s_step_layer);
  for (int i=0; i<12; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }
  
  health_service_events_unsubscribe();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  update_time();
}
static void deinit() {
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}