#include <pebble.h>

static Window *window;
static Layer *window_layer, *layer;
static GBitmap *bgBitmap;
static GBitmap *maskBitmap;
static BitmapLayer *maskLayer;
static GPoint center;
static struct tm now;


static void graphics_draw_thick_line(GContext *ctx, const GPoint p0, const GPoint p1, const int thick) {
	bool steep;
	int x0, y0, x1, y1, iTmp, deltax, deltay, error, ystep, x, y, xx0, yy0, xx1, yy1, thickOver2, xt;

	x0 = p0.x; y0 = p0.y;
	x1 = p1.x; y1 = p1.y;
	
	steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep) {
		// swap(x0, y0)
		iTmp = x0;
		x0 = y0;
		y0 = iTmp;

		// swap(x1, y1)
		iTmp = x1;
		x1 = y1;
		y1 = iTmp;
	}

	if (x0 > x1) {
		// swap(x0, x1)
		iTmp = x0;
		x0 = x1;
		x1 = iTmp;

		// swap(y0, y1)
		iTmp = y0;
		y0 = y1;
		y1 = iTmp;
	}

	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = deltax / 2;
	ystep = 0;
	y = y0;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	thickOver2 = thick/2;
	xt = x1-thick;
	for (x = x0; x <= x1; x++) {
		xx0 = x-thickOver2;
		yy0 = y-thickOver2;
		xx1 = xx0 + thick - 1;
		yy1 = yy0 + thick - 1;
		if (steep) {
			//graphics_draw_rect(ctx, GRect(y-thickOver2, x-thickOver2, thick, thick));
			graphics_draw_line(ctx, GPoint(yy0, xx0), GPoint(yy1, xx0));
			graphics_draw_line(ctx, GPoint(yy0, xx0), GPoint(yy0, xx1));
			if (x >= xt) {
				graphics_draw_line(ctx, GPoint(yy1, xx0), GPoint(yy1, xx1));
				graphics_draw_line(ctx, GPoint(yy0, xx1), GPoint(yy1, xx1));
			}
		} else {
			//graphics_draw_rect(ctx, GRect(x-thickOver2, y-thickOver2, thick, thick));
			graphics_draw_line(ctx, GPoint(xx0, yy0), GPoint(xx0, yy1));
			graphics_draw_line(ctx, GPoint(xx0, yy0), GPoint(xx1, yy0));
			if (x >= xt) {
				graphics_draw_line(ctx, GPoint(xx1, yy0), GPoint(xx1, yy1));
				graphics_draw_line(ctx, GPoint(xx0, yy1), GPoint(xx1, yy1));
			}
		}
		
		error = error - deltay;
		if (error < 0) {
			y = y + ystep;
			error = error + deltax;
		}
	}
}

static void updateLayer(Layer *layer, GContext *ctx) {
	int angle;
	int cos, sin;
	GPoint a;
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
		
	a.x = 72 + 72*sin/TRIG_MAX_RATIO;
	a.y = 84 - 72*cos/TRIG_MAX_RATIO;

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_thick_line(ctx, a, center, 5);

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, a, center);
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
