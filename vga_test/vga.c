#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>
#include <pico/scanvideo/scanvideo_base.h>
#include <stdio.h>
#include <stdlib.h>

#include "vga.h"

static inline uint16_t *
prepare_scanline_buffer(struct scanvideo_scanline_buffer *dest, uint width);

static inline void
finalize_scanline_buffer(struct scanvideo_scanline_buffer *dest);

uint16_t *get_canvas() {
  static uint16_t *canvas = NULL;

  // Initialize canvas if not already done
  if (canvas == NULL) {
    canvas = (uint16_t *)malloc(sizeof(uint16_t) * CANVAS_SIZE);
    if (canvas == NULL) {
      printf("Error: Memory allocation for canvas failed!\n");
      return NULL;
    }

    // Clear the canvas
    for (size_t i = 0; i < CANVAS_SIZE; i++) {
      canvas[i] = 0;
    }
  }

  return canvas;
}

uint16_t *get_next_canvas_slice(uint16_t *canvas) {
  static size_t current_row_index = 0;

  // Update logic: Cycle through rows
  current_row_index++;
  if (current_row_index >= VGA_MODE.height) {
    current_row_index = 0;
  }

  return &canvas[current_row_index * VGA_MODE.width];
}

void render_scanline(struct scanvideo_scanline_buffer *dest,
                     uint16_t *canvas_slice) {
  uint16_t *color_buffer = prepare_scanline_buffer(dest, VGA_MODE.width);

  // Copy the row from the canvas to the color buffer
  for (size_t px = 0; px < VGA_MODE.width; px++) {
    color_buffer[px] = canvas_slice[px];
  }

  finalize_scanline_buffer(dest);
}

// Canvas Manipulation Functions
void draw_rectangle_filled(uint16_t *canvas, size_t x, size_t y, size_t width,
                           size_t height, uint16_t color) {
  for (size_t row = 0; row < height; row++) {
    size_t canvas_y = y + row;
    if (canvas_y >= VGA_MODE.height)
      break;

    for (size_t col = 0; col < width; col++) {
      size_t canvas_x = x + col;
      if (canvas_x >= VGA_MODE.width)
        break;

      size_t index = canvas_y * VGA_MODE.width + canvas_x;
      canvas[index] = color;
    }
  }
}

// Draw a rectangle with borders only
void draw_rectangle_border(uint16_t *canvas, size_t x, size_t y, size_t width,
                           size_t height, uint16_t color) {
  // Top and bottom borders
  for (size_t col = 0; col < width; col++) {
    size_t top_index = y * VGA_MODE.width + (x + col);
    size_t bottom_index = (y + height - 1) * VGA_MODE.width + (x + col);

    if (x + col < VGA_MODE.width) {
      if (y < VGA_MODE.height)
        canvas[top_index] = color; // Top border
      if (y + height - 1 < VGA_MODE.height)
        canvas[bottom_index] = color; // Bottom border
    }
  }

  // Left and right borders
  for (size_t row = 0; row < height; row++) {
    size_t left_index = (y + row) * VGA_MODE.width + x;
    size_t right_index = (y + row) * VGA_MODE.width + (x + width - 1);

    if (y + row < VGA_MODE.height) {
      if (x < VGA_MODE.width)
        canvas[left_index] = color; // Left border
      if (x + width - 1 < VGA_MODE.width)
        canvas[right_index] = color; // Right border
    }
  }
}

// Helper Functions

static inline uint16_t *
prepare_scanline_buffer(struct scanvideo_scanline_buffer *dest, uint width) {
  assert(width >= 3 && width % 2 == 0);

  // Prepare composable scanline header
  dest->data[0] = COMPOSABLE_RAW_RUN | ((width + 1 - 3) << 16);
  dest->data[width / 2 + 2] = 0x0000u | (COMPOSABLE_EOL_ALIGN << 16);
  dest->data_used = width / 2 + 2;

  assert(dest->data_used <= dest->data_max);
  return (uint16_t *)&dest->data[1];
}

static inline void
finalize_scanline_buffer(struct scanvideo_scanline_buffer *dest) {
  uint32_t first = dest->data[0];
  uint32_t second = dest->data[1];
  dest->data[0] = (first & 0x0000ffffu) | ((second & 0x0000ffffu) << 16);
  dest->data[1] = (second & 0xffff0000u) | ((first & 0xffff0000u) >> 16);
  dest->status = SCANLINE_OK;
}
