#include "infrared.h"

#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/time.h>
#include <stdio.h>

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
      !IR_IN_TIMING_WINDOW(start_pulse_duration, IR_START_PULSE,
                           IR_TIMING_SLACK_US) ||
      !IR_IN_TIMING_WINDOW(start_space_duration, IR_START_SPACE,
                           IR_TIMING_SLACK_US)) {
    printf("Invalid start sequence. Message discarded.\n");
    event_count = 0;
    return;
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

