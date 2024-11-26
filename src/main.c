#include <stdio.h>

#include <pico.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/sync.h>

#if PICO_ON_DEVICE
#include <hardware/clocks.h>
#endif

#include "vga.h"

static struct mutex canvas_mutex; // Probably unnecessary
struct semaphore video_setup_complete;

void render_loop() {
  while (true) {
    // Begin scanline generation
    struct scanvideo_scanline_buffer *scanline_buffer =
        scanvideo_begin_scanline_generation(true);

    mutex_enter_blocking(&canvas_mutex);

    uint16_t *canvas = get_canvas();
    hard_assert(canvas != NULL);
    // Update canvas slice and render the scanline
    uint16_t *current_slice = get_next_canvas_slice(canvas);
    render_scanline(scanline_buffer, current_slice);

    mutex_exit(&canvas_mutex);

    // End scanline generation
    scanvideo_end_scanline_generation(scanline_buffer);
  }
}

int vga_main() {
  printf("VGA_SETUP");
  scanvideo_setup(&VGA_MODE);
  scanvideo_timing_enable(true);

  { (void)get_canvas(); } // force canvas initialization :^)

  sem_release(&video_setup_complete); // Setup complete, let core 1 start

  render_loop();
  return 0;
}

void core1_func() {
  sem_acquire_blocking(&video_setup_complete);

  uint16_t bg_color =
      (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0xc7, 0xff, 0xdd);
  uint16_t rect_color =
      (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0x42, 0xba, 0xff);

  uint16_t rect_x = 20;
  uint16_t rect_y = 20;
  uint16_t rect_w = 20;
  uint16_t rect_h = 20;

  uint16_t padding_x = 4;
  uint16_t padding_y = 10;

  int16_t v_x = 2;
  int16_t v_y = 2;

  absolute_time_t last_time = get_absolute_time();

  while (1) {
    if (rect_x + rect_w > VGA_MODE.width - padding_x) {
      rect_x = VGA_MODE.width - rect_w - padding_x; // Correct position
      v_x *= -1;                                  // Reverse velocity
    } else if (rect_x < padding_x) {
      rect_x = padding_x; // Correct position
      v_x *= -1;        // Reverse velocity
    }

    if (rect_y + rect_h > VGA_MODE.height - padding_y) {
      rect_y = VGA_MODE.height - rect_h - padding_y; // Correct position
      v_y *= -1;                                   // Reverse velocity
    } else if (rect_y < padding_y) {
      rect_y = padding_y; // Correct position
      v_y *= -1;        // Reverse velocity
    }
    printf("pos: %d, %d | v: %d, %d\n", rect_x, rect_y, v_x, v_y);

    mutex_enter_blocking(&canvas_mutex);

    uint16_t *canvas = get_canvas();
    draw_rectangle_filled(canvas, rect_x, rect_y, rect_w, rect_h, bg_color);
    rect_x += v_x;
    rect_y += v_y;
    draw_rectangle_filled(canvas, rect_x, rect_y, rect_w, rect_h, rect_color);

    mutex_exit(&canvas_mutex);

    absolute_time_t target_time =
        delayed_by_ms(last_time, 33); // 33 ms approx 30 FPS
    sleep_until(target_time);
    last_time = target_time;
  }
}

int main(void) {
#if PICO_SCANVIDEO_48MHZ
  set_sys_clock_48mhz();
#endif
  stdio_usb_init();
  setup_default_uart();

  sem_init(&video_setup_complete, 0, 1);
  mutex_init(&canvas_mutex);

  multicore_launch_core1(core1_func);
  return vga_main();
}
