#ifndef _VGA_H_
#define _VGA_H_

#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>
#include <pico/scanvideo/scanvideo_base.h>

#define VGA_MODE vga_mode_320x240_60
#define CANVAS_WIDTH VGA_MODE.width
#define CANVAS_HEIGHT VGA_MODE.height
#define CANVAS_SIZE (CANVAS_WIDTH * CANVAS_HEIGHT)

uint16_t *vga_get_canvas(void);
uint16_t *vga_get_next_canvas_slice(uint16_t *canvas);

void vga_init(void);

void vga_clear_canvas(uint16_t *canvas);

void vga_render_scanline(struct scanvideo_scanline_buffer *dest,
                         uint16_t *canvas_slice);

void vga_draw_rectangle_filled(uint16_t *canvas, size_t x, size_t y,
                               size_t width, size_t height, uint16_t color);

void vga_draw_rectangle_border(uint16_t *canvas, size_t x, size_t y,
                               size_t width, size_t height, uint16_t color);
#endif // _VGA_H_
