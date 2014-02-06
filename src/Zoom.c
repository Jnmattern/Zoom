#include <pebble.h>

static Window *window;
static Layer *window_layer, *layer;
static GBitmap *bgBitmap;
static GBitmap *maskBitmap;
static BitmapLayer *maskLayer;
static GPoint center;
static struct tm now;

static void updateLayer(Layer *layer, GContext *ctx) {
	int angle, i, j;
	int cos, sin;
	GPoint a, p1, p2;
	static GRect r = { { 0, 0}, { 144, 168 } };
	
	angle = TRIG_MAX_ANGLE * (60*(now.tm_hour%12) + now.tm_min) / 720;
	cos = cos_lookup(angle);
	sin = sin_lookup(angle);
		
	center.x = 72 - 72*sin/TRIG_MAX_RATIO;
	center.y = 84 + 72*cos/TRIG_MAX_RATIO;

	r.origin.x = center.x - bgBitmap->bounds.size.w/2;
	r.origin.y = center.y - bgBitmap->bounds.size.h/2;
	r.size = bgBitmap->bounds.size;

	graphics_draw_bitmap_in_rect(ctx, bgBitmap, r);
	
	graphics_context_set_stroke_color(ctx, GColorBlack);
	
	a.x = 72 + 72*sin/TRIG_MAX_RATIO;
	a.y = 84 - 72*cos/TRIG_MAX_RATIO;

	for (j=-1; j<2; j++) {
		for (i=-1; i<2; i++) {
			p1 = center;
			p2 = a;
			p1.x += i;
			p2.x += i;
			p1.y += j;
			p2.y += j;
			
			graphics_draw_line(ctx, p1, p2);
		}
	}

	graphics_context_set_stroke_color(ctx, GColorWhite);
	p1 = center;
	p2 = a;
	graphics_draw_line(ctx, p1, p2);
}

static void handleTick(struct tm *t, TimeUnits units_changed) {
	now = *t;	
	layer_mark_dirty(layer);
}

static void init(void) {
	time_t n = time(NULL);
	now = *localtime(&n);
	
	window = window_create();
	window_set_background_color(window, GColorWhite);
	window_stack_push(window, true);
	window_layer = window_get_root_layer(window);
	
	bgBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
	layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(layer, updateLayer);
	layer_add_child(window_layer, layer);
	
	maskBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MASK);
	maskLayer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(maskLayer, maskBitmap);
	bitmap_layer_set_compositing_mode(maskLayer, GCompOpAnd);
	layer_add_child(window_layer, bitmap_layer_get_layer(maskLayer));
	
	tick_timer_service_subscribe(MINUTE_UNIT, handleTick);
}

static void deinit(void) {
	tick_timer_service_unsubscribe();
	bitmap_layer_destroy(maskLayer);
	gbitmap_destroy(maskBitmap);
	gbitmap_destroy(bgBitmap);
	layer_destroy(layer);
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
