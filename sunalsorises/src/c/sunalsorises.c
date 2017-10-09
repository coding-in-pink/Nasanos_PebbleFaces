#include <pebble.h>
#include "dithering.h"
#include "ticks.h"

static Window *s_main_window;
static Layer *s_bg_layer, *s_canvas_layer;
static TextLayer *s_dateuno_layer, *s_datedos_layer, *s_step_layer;
static GFont s_date_font;

static GPath *s_tick_paths[12];
static GPath *s_mintick_paths[60];
static GPath *s_minute_arrow, *s_hour_arrow;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char dateuno_buffer[8];
  static char datedos_buffer[8];
  strftime(dateuno_buffer, sizeof(dateuno_buffer), "%a", tick_time);
  strftime(datedos_buffer, sizeof(datedos_buffer), "%d", tick_time);
  text_layer_set_text(s_dateuno_layer, dateuno_buffer);
  text_layer_set_text(s_datedos_layer, datedos_buffer);
  
  layer_mark_dirty(s_canvas_layer);
  //layer_mark_dirty(window_get_root_layer(s_main_window));
}

static void health_handler(HealthEventType event, void *context) {
  static char s_step_buffer[8];
  if (event == HealthEventMovementUpdate) {
    snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    text_layer_set_text(s_step_layer, s_step_buffer);
  }
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  GRect layer_bounds = layer_get_bounds(layer);
  GPoint layer_center = grect_center_point(&layer_bounds);
  uint16_t radius = layer_bounds.size.w/2;
  
  //Center face
  draw_gradient_rect(ctx, GRect(0, 0, layer_bounds.size.w, layer_bounds.size.h), GColorPictonBlue, GColorCobaltBlue, TOP_TO_BOTTOM);
  //draw_dithered_circle(ctx, layer_center.x, layer_center.y, radius-25, GColorBlack, GColorCobaltBlue, DITHER_50_PERCENT);
  
  //Center face rings
  graphics_context_set_stroke_color(ctx, GColorBlue);
  graphics_context_set_stroke_width(ctx, 1);
  for(int i=5; i<radius; i+=2) {
    graphics_draw_circle(ctx, layer_center, i);
  }
  
  //Outer face
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_stroke_width(ctx, 24);
  graphics_draw_circle(ctx, layer_center, radius-12);
  //graphics_context_set_fill_color(ctx, GColorLightGray);
  //graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  
  //Facial rings
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, layer_center, radius-24);
  
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, layer_center, radius-22);
  
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 6);
  graphics_draw_circle(ctx, layer_center, radius-4);
  
  graphics_context_set_stroke_width(ctx, 1);
  
  //Date and step frames
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(layer_center.x+layer_bounds.size.w/8, layer_center.y-18, 30, 36), 4, GCornersAll);
  graphics_draw_round_rect(ctx, GRect(layer_center.x+layer_bounds.size.w/8+1, layer_center.y-17, 30, 36), 4);
  
  graphics_fill_rect(ctx, GRect(layer_center.x-25, layer_center.y+30, 50, 24), 4, GCornersAll);
  graphics_draw_round_rect(ctx, GRect(layer_center.x-24, layer_center.y+31, 48, 22), 4);
  
  //Ticks
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  for (int i=0; i<12; ++i) {
    gpath_rotate_to(s_tick_paths[i], (TRIG_MAX_ANGLE * i / 12));
    gpath_draw_filled(ctx, s_tick_paths[i]);
    gpath_draw_outline(ctx, s_tick_paths[i]);
  }
  //Small ticks
  for (int i=0; i<60; ++i) {
    if (i==0 || i%5==0) {
      gpath_rotate_to(s_mintick_paths[i], (TRIG_MAX_ANGLE * i / 60));
    } else {
      gpath_rotate_to(s_mintick_paths[i], (TRIG_MAX_ANGLE * i / 60));
      gpath_draw_filled(ctx, s_mintick_paths[i]);
      gpath_draw_outline(ctx, s_mintick_paths[i]);
    }
  }
}
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  //Setup canvas
  GRect layer_bounds = layer_get_bounds(layer);
  GPoint layer_center = grect_center_point(&layer_bounds);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  //Secondhand
  //const int16_t second_hand_length = layer_bounds.size.w/2-20;
  //int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  //GPoint second_hand = {
  //  .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + layer_center.x,
  //  .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + layer_center.y,
  //};
  //graphics_draw_line(ctx, second_hand, layer_center);

  //Minutehand
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);

  //Hourhand
  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);
  
  //Center ring
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_circle(ctx, layer_center, 3);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, layer_center, 2);
  graphics_draw_circle(ctx, layer_center, 4);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  
  //Canvas set up
  s_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_bg_layer, bg_update_proc);
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  
  //Date set up
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WRTONWALL_18));
  s_dateuno_layer = text_layer_create(GRect(bounds.size.w/4-7, bounds.size.h/2-19, bounds.size.w, 30));
  text_layer_set_background_color(s_dateuno_layer, GColorClear);
  text_layer_set_text_color(s_dateuno_layer, GColorWhite);
  text_layer_set_font(s_dateuno_layer, s_date_font);
  text_layer_set_text_alignment(s_dateuno_layer, GTextAlignmentCenter);

  s_datedos_layer = text_layer_create(GRect(bounds.size.w/4-7, bounds.size.h/2-4, bounds.size.w, 30));
  text_layer_set_background_color(s_datedos_layer, GColorClear);
  text_layer_set_text_color(s_datedos_layer, GColorWhite);
  text_layer_set_font(s_datedos_layer, s_date_font);
  text_layer_set_text_alignment(s_datedos_layer, GTextAlignmentCenter);
  
  //Stepcount set up
  s_step_layer = text_layer_create(GRect(0, bounds.size.h/2+30, bounds.size.w, 30));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_text_color(s_step_layer, GColorWhite);
  text_layer_set_font(s_step_layer, s_date_font);
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  
  //Ticks set up
  for (int i=0; i<12; ++i) {
    s_tick_paths[i] = gpath_create(&TICK_POINTS);
    gpath_move_to(s_tick_paths[i], GPoint(((sin_lookup(TRIG_MAX_ANGLE*i/12)*(bounds.size.w/2-12)/TRIG_MAX_RATIO)+center.x),
                                         (-cos_lookup(TRIG_MAX_ANGLE*i/12)*(bounds.size.h/2-12)/TRIG_MAX_RATIO)+center.y));
  }
  //Small ticks set up
  for (int i=0; i<60; ++i) {
    s_mintick_paths[i] = gpath_create(&MIN_TICK_POINTS);
    if (i==0 || i%5==0) {
      gpath_move_to(s_mintick_paths[i], GPoint(bounds.size.w*20, bounds.size.h*20));
    } else {
      gpath_move_to(s_mintick_paths[i], GPoint(((sin_lookup(TRIG_MAX_ANGLE*i/60)*(bounds.size.w/2-12)/TRIG_MAX_RATIO)+center.x),
                                         (-cos_lookup(TRIG_MAX_ANGLE*i/60)*(bounds.size.h/2-12)/TRIG_MAX_RATIO)+center.y));
    }
  }
  
  //Minute and hour hand set up
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  gpath_move_to(s_minute_arrow, center);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  gpath_move_to(s_hour_arrow, center);
  
  //Health subscription
  if (health_service_events_subscribe(health_handler, NULL)) {
    health_handler(HealthEventMovementUpdate, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    text_layer_set_text(s_step_layer, "Error!");
  }
  
  layer_add_child(window_get_root_layer(window), s_bg_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_dateuno_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_datedos_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
}
static void main_window_unload(Window *window) {
  layer_destroy(s_bg_layer);
  layer_destroy(s_canvas_layer);
  
  fonts_unload_custom_font(s_date_font);
  text_layer_destroy(s_dateuno_layer);
  text_layer_destroy(s_datedos_layer);
  text_layer_destroy(s_step_layer);

  for (int i=0; i<12; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }
  for (int i=0; i<60; ++i) {
    gpath_destroy(s_mintick_paths[i]);
  }
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);
  
  health_service_events_unsubscribe();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
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