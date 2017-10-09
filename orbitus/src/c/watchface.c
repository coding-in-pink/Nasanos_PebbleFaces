#include <pebble.h>

static Window *win;
static Layer *faceLay;

static void tempUp() {
  layer_mark_dirty(faceLay);
}

static void faceUpdat(Layer *layer, GContext *ctx) {
  GRect faceLayB = layer_get_bounds(layer);
  GPoint faceLayC = grect_center_point(&faceLayB);
  uint16_t faceLayRad = faceLayB.size.w/2;
  
  time_t tempLog = time(NULL);
  struct tm *temp = localtime(&tempLog);
  
  //Hands and their rings
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  GPoint hP = GPoint(((sin_lookup(TRIG_MAX_ANGLE*temp->tm_hour/12)*(faceLayB.size.w/2-24)/TRIG_MAX_RATIO)+faceLayC.x),
                                         (-cos_lookup(TRIG_MAX_ANGLE*temp->tm_hour/12)*(faceLayB.size.h/2-24)/TRIG_MAX_RATIO)+faceLayC.y);
  GPoint mP = GPoint(((sin_lookup(TRIG_MAX_ANGLE*temp->tm_hour/60)*(faceLayB.size.w/2-12)/TRIG_MAX_RATIO)+faceLayC.x),
                                         (-cos_lookup(TRIG_MAX_ANGLE*temp->tm_hour/60)*(faceLayB.size.h/2-12)/TRIG_MAX_RATIO)+faceLayC.y);
  
  graphics_draw_circle(ctx, faceLayC, faceLayRad - 24);
  graphics_draw_circle(ctx, faceLayC, faceLayRad - 12);
  
  graphics_fill_circle(ctx, hP, 8);
  graphics_fill_circle(ctx, mP, 4);
}

static void loadWin(Window *window) {
  Layer *winLay = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(winLay);
  
  window_set_background_color(win, GColorDarkGray);
  
  faceLay = layer_create(bounds);
  layer_set_update_proc(faceLay, faceUpdat);
  
  layer_add_child(winLay, faceLay);
}

static void unloadWin(Window *window) {
  layer_destroy(faceLay);
}

static void tickHandle(struct tm *tick_time, TimeUnits units_changed) {
  tempUp();
}

static void init() {
  tick_timer_service_subscribe(MINUTE_UNIT, tickHandle);
  win = window_create();
  window_set_window_handlers(win, (WindowHandlers) {
    .load = loadWin,
    .unload = unloadWin
  });
  
  window_stack_push(win, true);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(win);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}