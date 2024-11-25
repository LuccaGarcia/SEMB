
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include <stdio.h>

#define GPIO_PIN 22 // GPIO pin connected to the IR receiver
#define PULSE_TIME                                                             \
  562 // Pulse time in microseconds (approximate for NEC protocol)
#define START_PULSE 9000               // Start pulse duration (9ms)
#define START_SPACE 4500               // Start space duration (4.5ms)
#define LOGIC_0_SPACE (2 * PULSE_TIME) // Space duration for logic 0
#define LOGIC_1_SPACE (4 * PULSE_TIME) // Space duration for logic 1
#define BUFFER_SIZE 100                // Maximum number of events to store

// Structure to store events
typedef struct {
  uint64_t timestamp; // Timestamp of the falling edge
} ir_event_t;

volatile ir_event_t event_buffer[BUFFER_SIZE]; // Buffer to store events
volatile int event_count = 0;                  // Number of events stored

// Function to decode NEC protocol from buffered events
void decode_nec_protocol() {
  if (event_count < 2)
    return; // Not enough events to decode

  printf("Decoding NEC Protocol:\n");
  uint64_t start = event_buffer[1].timestamp - event_buffer[0].timestamp;
  uint64_t space = event_buffer[3].timestamp - event_buffer[2].timestamp;

  printf("Start: %llu us\n", start);
  printf("Space: %llu us\n", space);

  event_count = 0;
}

// GPIO interrupt callback
void gpio_callback(uint gpio, uint32_t events) {
  if (event_count < BUFFER_SIZE) {
    if (events & GPIO_IRQ_EDGE_FALL || events & GPIO_IRQ_EDGE_RISE) {
      // Record falling edge timestamp
      event_buffer[event_count].timestamp = time_us_64();
      event_count++;
    }
  } else {
    printf("Buffer overflow! Clearing buffer.\n");
    event_count = 0;
  }
}

// Timer callback to process and decode events after inactivity
bool timer_callback(repeating_timer_t *rt) {
  static uint64_t last_event_time = 0;
  uint64_t current_time = time_us_64();

  if (event_count > 0) {
    uint64_t latest_event_time = event_buffer[event_count - 1].timestamp;
    if (current_time - latest_event_time > 10000) { // 10ms inactivity
      decode_nec_protocol(); // Decode the buffered events
      event_count = 0;       // Reset the buffer
    }
    last_event_time = latest_event_time;
  }

  return true; // Continue running the timer
}

int main() {
  stdio_init_all(); // Initialize standard I/O for printf

  gpio_init(GPIO_PIN);             // Initialize the GPIO pin
  gpio_set_dir(GPIO_PIN, GPIO_IN); // Set GPIO as input
  gpio_pull_up(GPIO_PIN);          // Enable pull-up resistor (optional)

  // Enable interrupts on falling edges for the specified GPIO
  gpio_set_irq_enabled_with_callback(
      GPIO_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

  // Start a repeating timer to check for inactivity
  repeating_timer_t timer;
  add_repeating_timer_us(-1000, timer_callback, NULL, &timer); // 1ms interval

  while (1) {
    tight_loop_contents(); // Keeps the main loop running
  }

  return 0;
}
