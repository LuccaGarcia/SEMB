// C includes
#include <stdio.h>

// Pico SDK
#include <pico.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <stdlib.h>
#include <task.h>

// Project specific
#include "game.h"
#include "vga.h"

#define mainGAME_LOGIC_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

static void prvSetupHardware(void);
static void prvLaunchRTOS();

static struct mutex render_sync_mutex; // Probably unnecessary

void render_loop() {
  vga_init();

  while (true) {
    // Begin scanline generation
    struct scanvideo_scanline_buffer *scanline_buffer =
        scanvideo_begin_scanline_generation(true);

    mutex_enter_blocking(&render_sync_mutex);

    uint16_t *canvas = vga_get_canvas();

    // Update canvas slice and render the scanline
    uint16_t *current_slice = vga_get_next_canvas_slice(canvas);
    vga_render_scanline(scanline_buffer, current_slice);

    mutex_exit(&render_sync_mutex);

    // End scanline generation
    scanvideo_end_scanline_generation(scanline_buffer);
  }
}

void update_canvas(const struct game_state *gs) {
  mutex_enter_blocking(&render_sync_mutex);
  uint16_t *canvas = vga_get_canvas();

  // Clear the old rectangle
  vga_draw_rectangle_filled(canvas, gs->rect_x - gs->v_x, gs->rect_y - gs->v_y,
                            gs->rect_w, gs->rect_h, gs->bg_color);

  // Draw the new rectangle
  vga_draw_rectangle_filled(canvas, gs->rect_x, gs->rect_y, gs->rect_w,
                            gs->rect_h, gs->rect_color);

  mutex_exit(&render_sync_mutex);
}

static void prvGameLogicTask(void *pvParameters) {
  (void)pvParameters; // Not used by task

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
      .canvas_w = CANVAS_WIDTH,
      .canvas_h = CANVAS_HEIGHT,
  };

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(33);

  for (;;) {
    printf("logic: %u, %u\n", xLastWakeTime, xFrequency);

    // Update game state
    update_game_state(&gs);

    // Draw the updated state to the canvas
    update_canvas(&gs);

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

int main(void) {
  prvSetupHardware();

  mutex_init(&render_sync_mutex);
  multicore_launch_core1(render_loop);

  xTaskCreate(prvGameLogicTask, "Game Logic", configMINIMAL_STACK_SIZE, NULL,
              mainGAME_LOGIC_TASK_PRIORITY, NULL);

  prvLaunchRTOS();
}

static void prvSetupHardware(void) {
  /* Want to be able to printf */
  stdio_usb_init();
}

static void prvLaunchRTOS() {
  printf("Core %d: Launching FreeRTOS scheduler\n", get_core_num());
  /* Start the tasks and timer running. */
  vTaskStartScheduler();
  /* should never reach here */
  panic_unsupported();
}
