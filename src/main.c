#include <pico.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <stdio.h>

#include "game.h"
#include "vga.h"

static struct mutex canvas_mutex; // Probably unnecessary
struct semaphore video_setup_complete;

void render_loop() {
  sem_acquire_blocking(&video_setup_complete);

  // TODO: clean up this logic into VGA module
  while (true) {
    // Begin scanline generation
    struct scanvideo_scanline_buffer *scanline_buffer =
        scanvideo_begin_scanline_generation(true);

    mutex_enter_blocking(&canvas_mutex);

    uint16_t *canvas = vga_get_canvas();

    hard_assert(canvas != NULL);
    // Update canvas slice and render the scanline
    uint16_t *current_slice = vga_get_next_canvas_slice(canvas);
    vga_render_scanline(scanline_buffer, current_slice);

    mutex_exit(&canvas_mutex);

    // End scanline generation
    scanvideo_end_scanline_generation(scanline_buffer);
  }
}

void update_canvas(const struct game_state *gs) {

  mutex_enter_blocking(&canvas_mutex);
  uint16_t *canvas = vga_get_canvas();

  // Clear the old rectangle
  vga_draw_rectangle_filled(canvas, gs->rect_x - gs->v_x, gs->rect_y - gs->v_y,
                            gs->rect_w, gs->rect_h, gs->bg_color);

  // Draw the new rectangle
  vga_draw_rectangle_filled(canvas, gs->rect_x, gs->rect_y, gs->rect_w,
                            gs->rect_h, gs->rect_color);
  mutex_exit(&canvas_mutex);
}

bool game_logic_callback(repeating_timer_t *timer) {
  // Some metrics
  {
    static absolute_time_t last_tick = 0;
    absolute_time_t now = get_absolute_time();
    absolute_time_t delta = now - last_tick;
    last_tick = now;
    printf("logic period: %llu us\n", delta);
  }

  struct game_state *gs = (struct game_state *)timer->user_data;

  // Update game state
  update_game_state(gs);

  // Draw the updated state to the canvas
  update_canvas(gs);

  return true; // Returning true keeps the timer running
}

int main(void) {
  stdio_usb_init();
  mutex_init(&canvas_mutex);

  sem_init(&video_setup_complete, 0, 1);
  {
    multicore_launch_core1(render_loop);
    vga_init();
  }
  sem_release(
      &video_setup_complete); // VGA complete, let core 1 start rendering

  struct game_state gs = {
      .bg_color = (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0xc7, 0xff, 0xdd),
      .rect_color = (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0x42, 0xba, 0xff),
      .padding_x = 4,
      .padding_y = 10,
      .rect_x = 20,
      .rect_y = 20,
      .rect_w = 20,
      .rect_h = 20,
      .v_x = 2,
      .v_y = 2,
      .canvas_w = VGA_MODE.width,
      .canvas_h = VGA_MODE.height,
  };

  // Set up a repeating timer that calls the game_timer_callback every 33ms (~30
  // FPS)
  repeating_timer_t game_timer;
  if (!add_repeating_timer_ms(33, game_logic_callback, &gs, &game_timer)) {
    printf("Failed to add repeating timer!\n");
    return -1;
  }

  // The main thread can now perform other tasks or sleep
  while (1) {
    tight_loop_contents(); // Keeps the core idle and responsive
  }
}
