#include <pico.h>
#include <pico/stdlib.h>

#if PICO_ON_DEVICE
#include <hardware/clocks.h>
#endif

#include "vga.h"

void render_loop() {
  uint16_t canvas[CANVAS_SIZE];
  initialize_canvas(canvas);

  // Draw a rectangle (RGB5 Color: 0x40c0ff)
  // Demo rectangle
  uint16_t color1 = (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0x40, 0xc0, 0xff);
  uint16_t color2 = (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0x44, 0xc4, 0xff);

  draw_rectangle_filled(canvas, 20, 40, 50, 30, color1);
  draw_rectangle_border(canvas, 50, 50, 50, 30, color2);

  draw_rectangle_filled(canvas, 100, 100, 50, 30, color1);
  draw_rectangle_border(canvas, 100, 150, 50, 30, color2);

  while (true) {
    // Begin scanline generation
    struct scanvideo_scanline_buffer *scanline_buffer =
        scanvideo_begin_scanline_generation(true);

    // Update canvas slice and render the scanline
    uint16_t *current_slice = get_next_canvas_slice(canvas);
    render_scanline(scanline_buffer, current_slice);

    // End scanline generation
    scanvideo_end_scanline_generation(scanline_buffer);
  }
}

int vga_main(void) {
  scanvideo_setup(&VGA_MODE);
  scanvideo_timing_enable(true);
  render_loop();
  return 0;
}

int main(void) {
#if PICO_SCANVIDEO_48MHZ
  set_sys_clock_48mhz();
#endif
  setup_default_uart();
  return vga_main();
}
