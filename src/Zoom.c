#include <pebble.h>

static Window *window;
static Layer *window_layer, *layer;
static GBitmap *bgBitmap;
static GBitmap *maskBitmap;
static BitmapLayer *maskLayer;
static GPoint center;
static struct tm now;

#define OFFSET 5

/*
static void updateLayer(Layer *layer, GContext *ctx) {
	int angle, m, i, j;
	int cos, sin, c, s;
	GPoint a, b, p1, p2, tPos;
	GSize tSize;
	static char t[] = "xx";
	GFont font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
	GRect fontRect = GRect(0, 0, 100, 100);
	GRect textRect;
	
	//angle = TRIG_MAX_ANGLE * (60*(now.tm_hour%12) + now.tm_min) / 720;
	angle = TRIG_MAX_ANGLE * now.tm_sec / 60;
	cos = cos_lookup(angle);
	sin = sin_lookup(angle);
		
	center.x = 72 - 72*sin/TRIG_MAX_RATIO;
	center.y = 84 + 72*cos/TRIG_MAX_RATIO;

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_text_color(ctx, GColorBlack);
	
	for (m=0; m<720; m+=5) {
		angle = (TRIG_MAX_ANGLE * m / 720 + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE;
		c = cos_lookup(angle);
		s = sin_lookup(angle);
		
		b.x = center.x + 110*c/TRIG_MAX_RATIO;
		b.y = center.y + 110*s/TRIG_MAX_RATIO;
		
		if (!(m%60)) {
			int h = m/60;
			a.x = center.x + 90*c/TRIG_MAX_RATIO;
			a.y = center.y + 90*s/TRIG_MAX_RATIO;
			
			snprintf(t, sizeof(t), "%d", (h==0)?12:h);
			tSize = graphics_text_layout_get_content_size(t, font, fontRect, GTextOverflowModeWordWrap, GTextAlignmentLeft);
			if ((h == 11) || (h == 0) || (h == 1)) {
				tPos.x = a.x - tSize.w/2;
				tPos.y = a.y - OFFSET;
			} else if ((h == 2) || (h == 3) || (h == 4)) {
				tPos.x = a.x - tSize.w - OFFSET;
				tPos.y = a.y - tSize.h/2 - OFFSET;
			} else if ((h == 5) || (h == 6) || (h == 7)) {
				tPos.x = a.x - tSize.w/2;
				tPos.y = a.y - tSize.h - OFFSET;
			} else {
				tPos.x = a.x + OFFSET;
				tPos.y = a.y - tSize.h/2;
			}
			textRect.origin = tPos;
			textRect.size = tSize;
			graphics_draw_text(ctx, t, font, textRect, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
		} else if (!(m%30)) {
			a.x = center.x + 100*c/TRIG_MAX_RATIO;
			a.y = center.y + 100*s/TRIG_MAX_RATIO;
		} else if (!(m%15)) {
			a.x = center.x + 105*c/TRIG_MAX_RATIO;
			a.y = center.y + 105*s/TRIG_MAX_RATIO;
		} else {
			a.x = center.x + 109*c/TRIG_MAX_RATIO;
			a.y = center.y + 109*s/TRIG_MAX_RATIO;
		}
		graphics_draw_line(ctx, a, b);		
	}
	
	a.x = 72 - 72*sin/TRIG_MAX_RATIO;
	a.y = 84 + 72*cos/TRIG_MAX_RATIO;
	b.x = 72 + 72*sin/TRIG_MAX_RATIO;
	b.y = 84 - 72*cos/TRIG_MAX_RATIO;

	for (j=-1; j<2; j++) {
		for (i=-1; i<2; i++) {
			p1 = a;
			p2 = b;
			p1.x += i;
			p2.x += i;
			p1.y += j;
			p2.y += j;
			
			graphics_draw_line(ctx, p1, p2);
		}
	}
}
*/

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
