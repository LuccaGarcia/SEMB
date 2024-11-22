/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico.h>
#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>
#include <pico/stdlib.h>

#if PICO_ON_DEVICE
#include <hardware/clocks.h>
#endif

#define VGA_MODE vga_mode_320x240_60
extern const struct scanvideo_pio_program video_24mhz_composable;

static void frame_update_logic();
static void render_scanline(struct scanvideo_scanline_buffer *dest);

void render_loop() {
  while (true) {
    struct scanvideo_scanline_buffer *scanline_buffer =
        scanvideo_begin_scanline_generation(true);

    frame_update_logic();
    render_scanline(scanline_buffer);

    // Release the rendered buffer into the wild
    scanvideo_end_scanline_generation(scanline_buffer);
  }
}

int vga_main(void) {
  scanvideo_setup(&VGA_MODE);
  scanvideo_timing_enable(true);

  render_loop();
  return 0;
}

void frame_update_logic() {}

uint32_t to_scline_buffer(uint32_t low_half_word, uint32_t high_half_word) {
  return low_half_word | (high_half_word << 16);
}

int32_t single_color_scanline(uint32_t *buf, size_t buf_length, int width,
                              uint32_t color16) {
#define MIN_COLOR_RUN 3

  assert(buf_length >= 2);
  assert(width >= MIN_COLOR_RUN);

  uint32_t color_run_count = width - MIN_COLOR_RUN;
  uint32_t color_black = 0;

  // Select COLOR_RUN mode and repeat color16 for every pixel.
  // Afterwards, switch to RAW_1P mode and fill last pixel as black (mandatory).
  // Finaly signal end of scanline with EOL_ALIGN.
  buf[0] = to_scline_buffer(COMPOSABLE_COLOR_RUN, color16);
  buf[1] = to_scline_buffer(color_run_count, COMPOSABLE_RAW_1P);
  buf[2] = to_scline_buffer(color_black, COMPOSABLE_EOL_ALIGN);

  return 3; // return how much of the buffer was used.

#undef MIN_COLOR_RUN
}

/// WARN: NEEDS TESTING
int32_t scanline_from_raw_colors(uint32_t *buf, size_t buf_length,
                                 uint32_t *colors, size_t colors_length) {
#define MIN_COLOR_RUN 3

  assert(buf_length >= MIN_COLOR_RUN);
  assert(2 *buf_length = colors_length);

  uint32_t color_black = 0;

  buf[0] = to_scline_buffer(COMPOSABLE_RAW_RUN, colors[0]);
  buf[1] = to_scline_buffer(colors_length - 3, colors[1]);

  size_t buf_idx = 2;
  size_t col_idx = 2;

  while (buf_idx < buf_length - 1) {
    assert(col_idx < colors_length);
    buf[buf_idx] = to_scline_buffer(colors[col_idx], colors[col_idx + 1]);
    buf_idx++;
    col_idx += 2;
  }
  buf[buf_idx] = to_scline_buffer(COMPOSABLE_RAW_1P_SKIP_ALIGN, color_black);

  return buf_length;
#undef MIN_COLOR_RUN
}

void render_scanline(struct scanvideo_scanline_buffer *dest) {
  uint32_t *buf = dest->data;
  size_t buf_length = dest->data_max;

  int l = scanvideo_scanline_number(dest->scanline_id);
  uint16_t bgcolour = (uint16_t)l << 2;
  dest->data_used =
      single_color_scanline(buf, buf_length, VGA_MODE.width, bgcolour);
  dest->status = SCANLINE_OK;
}

int main(void) {
#if PICO_SCANVIDEO_48MHZ
  set_sys_clock_48mhz();
#endif
  // Re init uart now that clk_peri has changed
  setup_default_uart();

  return vga_main();
}
