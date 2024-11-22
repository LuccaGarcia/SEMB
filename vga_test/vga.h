#ifndef _VGA_H_
#define _VGA_H_

#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>
#include <pico/scanvideo/scanvideo_base.h>

#define VGA_MODE vga_mode_320x240_60
#define CANVAS_SIZE (VGA_MODE.width * VGA_MODE.height)
extern const struct scanvideo_pio_program video_24mhz_composable;

void initialize_canvas(uint16_t *canvas);
uint16_t *get_next_canvas_slice(uint16_t *canvas);

void render_scanline(struct scanvideo_scanline_buffer *dest,
                     uint16_t *canvas_slice);

void draw_rectangle_filled(uint16_t *canvas, size_t x, size_t y, size_t width,
                           size_t height, uint16_t color);

void draw_rectangle_border(uint16_t *canvas, size_t x, size_t y, size_t width,
                           size_t height, uint16_t color);
#endif // _VGA_H_
