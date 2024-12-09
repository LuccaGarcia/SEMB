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
#include <timers.h>

// Project specific
#include "game.h"
#include "infrared.h"
#include "vga.h"

#define mainGAME_LOGIC_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

volatile ir_event_t event_buffer[IR_BUFFER_SIZE]; // Buffer to store events
volatile uint16_t event_count = 0;                // Number of events stored
volatile uint8_t ir_command = 0;

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


  // Clear the old ball
  vga_draw_rectangle_filled(canvas, gs->Ball.x - gs->Ball.v_x,
                            gs->Ball.y - gs->Ball.v_y, gs->Ball.w, gs->Ball.h,
                            gs->bg_color);

  // Clear the old player 
  vga_draw_rectangle_filled(canvas, gs->Player.x, gs->Player.y, gs->Player.w,
                            gs->Player.h, gs->bg_color);

  // Clear the old AI
  vga_draw_rectangle_filled(canvas, gs->AI.x, gs->AI.y, gs->AI.w,
                            gs->AI.h, gs->bg_color);                      

  // Draw the new ball
  vga_draw_rectangle_filled(canvas, gs->Ball.x, gs->Ball.y, gs->Ball.w,
                            gs->Ball.h, gs->Ball.color);

  // Draw the new player
  vga_draw_rectangle_filled(canvas, gs->Player.x, gs->Player.y, gs->Player.w,
                            gs->Player.h, gs->Player.color);
  
  // Draw the new AI
  vga_draw_rectangle_filled(canvas, gs->AI.x, gs->AI.y, gs->AI.w,
                            gs->AI.h, gs->AI.color);

  mutex_exit(&render_sync_mutex);
}

static void prvGameLogicTask(void *pvParameters) {
  (void)pvParameters; // Not used by task
  uint16_t ball_color =
      (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0x42, 0xba, 0xff);

  uint16_t player_color =
      (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0xAC, 0x11, 0x22);
  
  uint16_t AI_color =
      (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0xDC, 0x01, 0x29);

  uint16_t bg_color_1 =
      (uint16_t)PICO_SCANVIDEO_PIXEL_FROM_RGB5(0xc7, 0xff, 0xdd);


  struct pong_rect ball = {
      .x = 20,
      .y = 20,
      .w = 20,
      .h = 20,
      .color = ball_color,
      .v_x = 2,
      .v_y = 2,
  };

  struct pong_rect player = {
      .x = 60,
      .y = 60,
      .w = 20,
      .h = 80,
      .color = player_color,
      .v_x = 2,
      .v_y = 2,
  };

  struct pong_rect AI = {
      .x = 220,
      .y = 220,
      .w = 20,
      .h = 80,
      .color = AI_color,
      .v_x = 2,
      .v_y = 2,
  };
  

  struct game_state gs = {
      .bg_color = bg_color_1,
      .padding_x = 4,
      .padding_y = 10,
      .Ball = ball,
      .Player = player,
      .AI = AI,
      .canvas_w = CANVAS_WIDTH,
      .canvas_h = CANVAS_HEIGHT,
  };

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(33);

  for (;;) {
    // Update game state
    update_game_state(&gs);
//    if (ir_command == IR_C_RIGHT) {
//      gs.rect_color = rect_color_1;
//      gs.bg_color = bg_color_1;
//    } else if (ir_command == IR_C_LEFT) {
//      gs.rect_color = rect_color_2;
//      gs.bg_color = bg_color_2;
//    } else if (ir_command == IR_C_UP) {
//      gs.v_x *= -1;
//      ir_command = IR_C_OK;
//    } else if (ir_command == IR_C_DOWN) {
//      gs.v_y *= -1;
//      ir_command = IR_C_OK;
//    }

    // Draw the updated state to the canvas
    update_canvas(&gs);

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// Function to decode NEC protocol from buffered events
uint32_t process_ir_buffer() {
  if (event_count < 3) {
    printf("Insufficient events to decode\n");
    event_count = 0;
    return 0;
  }

  // Validate start sequence
  uint64_t start_pulse_duration =
      event_buffer[1].timestamp - event_buffer[0].timestamp;
  uint64_t start_space_duration =
      event_buffer[2].timestamp - event_buffer[1].timestamp;

  if (!(event_buffer[0].event_kind & GPIO_IRQ_EDGE_FALL) ||
      !(event_buffer[1].event_kind & GPIO_IRQ_EDGE_RISE) ||
      !(event_buffer[2].event_kind & GPIO_IRQ_EDGE_FALL) ||
      !IR_IN_TIMING_WINDOW(start_pulse_duration, IR_START_PULSE,
                           IR_TIMING_SLACK_US) ||
      !IR_IN_TIMING_WINDOW(start_space_duration, IR_START_SPACE,
                           IR_TIMING_SLACK_US)) {
    event_count = 0;
    printf("Invalid start sequence. Message discarded.\n");
    return 0;
  }

  // Decode data bits
  uint32_t message = 0;
  uint8_t bit_count = 0;
  size_t last_fall_index = 2;

  for (size_t i = 3; i < event_count; i++) {
    if (bit_count == IR_MESSAGE_BIT_MAX) {
      break; // Stop decoding once all bits are read
    }

    if (event_buffer[i].event_kind & GPIO_IRQ_EDGE_RISE) {
      continue; // Ignore rising edges
    }

    uint64_t delta =
        event_buffer[i].timestamp - event_buffer[last_fall_index].timestamp;
    last_fall_index = i;

    if (IR_IN_TIMING_WINDOW(delta, IR_LOGIC_1_SPACE, IR_TIMING_SLACK_US)) {
      message = (message << 1) | 1; // Append logic 1
    } else if (IR_IN_TIMING_WINDOW(delta, IR_LOGIC_0_SPACE,
                                   IR_TIMING_SLACK_US)) {
      message = (message << 1); // Append logic 0
    } else {
      event_count = 0;
      printf("Invalid timing (%llu us). Message discarded.\n", delta);
      return 0;
    }

    bit_count++;
  }

  // Print decoded data
  event_count = 0; // Reset buffer
  return message;
}

// GPIO interrupt callback
void gpio_callback(uint gpio, uint32_t events) {
  (void)gpio;
  if (event_count >= IR_BUFFER_SIZE) {
    printf("Buffer overflow! Clearing buffer.\n");
    event_count = 0;
    return;
  }
  event_buffer[event_count].event_kind = events;
  event_buffer[event_count].timestamp = time_us_64();
  event_count++;
}

// Timer callback to process and decode events after inactivity
static void vDecodeTimerCallback(TimerHandle_t xTimer) {
  (void)xTimer;

  uint64_t current_time = time_us_64();

  if (event_count > 0) {
    uint64_t latest_event_time = event_buffer[event_count - 1].timestamp;
    if (current_time - latest_event_time > IR_TIMEOUT_MS) { // 10ms inactivity
      uint32_t raw_message = process_ir_buffer(); // Decode the buffered events
      if (!raw_message) {
        return;
      }

      uint8_t cmd_inv = (raw_message) & 0xFF;
      uint8_t command = (raw_message >> 8) & 0xFF;
      uint8_t addr_inv = (raw_message >> 16) & 0xFF;
      uint8_t addr = (raw_message >> 24) & 0xFF;

      bool cmd_valid = ((command ^ cmd_inv) == 0xFF);
      bool addr_valid = ((addr ^ addr_inv) == 0xFF);

      if (cmd_valid && addr_valid) {
        ir_command = command;
      }
    }
  }
}

int main(void) {
  TimerHandle_t xIrDecodeTimer = NULL;

  prvSetupHardware();

  mutex_init(&render_sync_mutex);
  multicore_launch_core1(render_loop);

  xTaskCreate(prvGameLogicTask, "Game Logic", configMINIMAL_STACK_SIZE, NULL,
              mainGAME_LOGIC_TASK_PRIORITY, NULL);

  TickType_t timer_period = pdMS_TO_TICKS(25);
  xIrDecodeTimer = xTimerCreate((const char *)"IrDecodeTimer", timer_period,
                                pdTRUE, (void *)0, vDecodeTimerCallback);
  xTimerStart(xIrDecodeTimer, 0);

  prvLaunchRTOS();
}

static void prvSetupHardware(void) {
  /* Want to be able to printf */
  stdio_usb_init();

  gpio_init(IR_GPIO_PIN);             // Initialize the GPIO pin
  gpio_set_dir(IR_GPIO_PIN, GPIO_IN); // Set GPIO as input
  gpio_pull_up(IR_GPIO_PIN);          // Enable pull-up resistor (optional)

  // Enable interrupts on falling edges for the specified GPIO
  gpio_set_irq_enabled_with_callback(IR_GPIO_PIN,
                                     GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
                                     true, &gpio_callback);
}

static void prvLaunchRTOS() {
  vTaskStartScheduler();
  /* should never reach here */
  panic_unsupported();
}
