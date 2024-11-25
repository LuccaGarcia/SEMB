#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include <stdio.h>

#define GPIO_PIN 22                    // GPIO pin connected to the IR receiver
#define PULSE_TIME 562                 // Single data pulse duration
#define START_PULSE 9300               // Start pulse duration (9ms)
#define START_SPACE 4500               // Start space duration (4.5ms)
#define IR_TIMEOUT_MS 10000            // Timeout between messages
#define IR_TIMER_PERIOD 1000           // Period of the IR timeout timer
#define LOGIC_0_SPACE (2 * PULSE_TIME) // Space duration for logic 0
#define LOGIC_1_SPACE (4 * PULSE_TIME) // Space duration for logic 1
#define BUFFER_SIZE 100                // Maximum number of events to store
#define MESSAGE_BIT_MAX 31

#define TIMING_SLACK_US 200
#define IN_TIMING_WINDOW(X, EXPECTED, SLACK)                                   \
  (EXPECTED + SLACK > X && X > EXPECTED - SLACK)

// Structure to store events
typedef struct {
  uint32_t event_kind;
  uint64_t timestamp;
} ir_event_t;

typedef enum {
  IDLE,
  START_LOW,
  START_HIGH,
  DATA,
} ir_phases;

volatile ir_phases ir_current_phase;
volatile ir_event_t event_buffer[BUFFER_SIZE]; // Buffer to store events
volatile int event_count = 0;                  // Number of events stored

// Function to decode NEC protocol from buffered events
void decode_nec_protocol() {
  if (event_count < 3) {
    printf("Insufficient events to decode\n");
    event_count = 0;
    return;
  }

  // Validate start sequence
  uint64_t start_pulse_duration =
      event_buffer[1].timestamp - event_buffer[0].timestamp;
  uint64_t start_space_duration =
      event_buffer[2].timestamp - event_buffer[1].timestamp;

  if (!(event_buffer[0].event_kind & GPIO_IRQ_EDGE_FALL) ||
      !(event_buffer[1].event_kind & GPIO_IRQ_EDGE_RISE) ||
      !(event_buffer[2].event_kind & GPIO_IRQ_EDGE_FALL) ||
      !IN_TIMING_WINDOW(start_pulse_duration, START_PULSE, TIMING_SLACK_US) ||
      !IN_TIMING_WINDOW(start_space_duration, START_SPACE, TIMING_SLACK_US)) {
    printf("Invalid start sequence. Message discarded.\n");
    event_count = 0;
    return;
  }

  // Decode data bits
  uint32_t message = 0;
  uint8_t bit_count = 0;
  size_t last_fall_index = 2;

  for (size_t i = 3; i < event_count; i++) {
    if (bit_count == MESSAGE_BIT_MAX) {
      break; // Stop decoding once all bits are read
    }

    if (event_buffer[i].event_kind & GPIO_IRQ_EDGE_RISE) {
      continue; // Ignore rising edges
    }

    uint64_t delta =
        event_buffer[i].timestamp - event_buffer[last_fall_index].timestamp;
    last_fall_index = i;

    if (IN_TIMING_WINDOW(delta, LOGIC_1_SPACE, TIMING_SLACK_US)) {
      message = (message << 1) | 1; // Append logic 1
    } else if (IN_TIMING_WINDOW(delta, LOGIC_0_SPACE, TIMING_SLACK_US)) {
      message = (message << 1); // Append logic 0
    } else {
      printf("Invalid timing (%llu us). Message discarded.\n", delta);
      event_count = 0;
      return;
    }

    bit_count++;
  }

  // Print decoded data
  printf("Decoded data: 0x%08X (%d bits)\n", message, bit_count);
  event_count = 0; // Reset buffer
}

// GPIO interrupt callback
void gpio_callback(uint gpio, uint32_t events) {
  if (event_count >= BUFFER_SIZE) {
    printf("Buffer overflow! Clearing buffer.\n");
    event_count = 0;
    return;
  }
  event_buffer[event_count].event_kind = events;
  event_buffer[event_count].timestamp = time_us_64();
  event_count++;
}

// Timer callback to process and decode events after inactivity
bool timer_callback(repeating_timer_t *rt) {
  static uint64_t last_event_time = 0;
  uint64_t current_time = time_us_64();

  if (event_count > 0) {
    uint64_t latest_event_time = event_buffer[event_count - 1].timestamp;
    if (current_time - latest_event_time > IR_TIMEOUT_MS) { // 10ms inactivity
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
  add_repeating_timer_us(-IR_TIMER_PERIOD, timer_callback, NULL, &timer);

  while (1) {
    tight_loop_contents(); // Keeps the main loop running
  }

  return 0;
}
