#ifndef _IR_H_
#define _IR_H_

#include <pico.h>

// -------- Commands --------

#define IR_C_OK 2
#define IR_C_RIGHT 194
#define IR_C_LEFT 94
#define IR_C_UP 0x00FF629D
#define IR_C_DOWN 0x00FFA857

#define IR_C_N1 0x00FF6897
#define IR_C_N2 0x00FF9867
#define IR_C_N3 0x00FFB04F
#define IR_C_N4 0x00FF30CF
#define IR_C_N5 0x00FF18E7
#define IR_C_N6 0x00FF7A85
#define IR_C_N7 0x00FF10EF
#define IR_C_N8 0x00FF38C7
#define IR_C_N9 0x00FF5AA5

// -------- Commands --------

#define IR_GPIO_PIN 20       // GPIO pin connected to the IR receiver
#define IR_PULSE_TIME 562    // Single data pulse duration
#define IR_START_PULSE 9300  // Start pulse duration (9ms)
#define IR_START_SPACE 4500  // Start space duration (4.5ms)
#define IR_TIMEOUT_MS 10000  // Timeout between messages
#define IR_TIMER_PERIOD 1000 // Period of the IR timeout timer
#define IR_LOGIC_0_SPACE (2 * IR_PULSE_TIME) // Space duration for logic 0
#define IR_LOGIC_1_SPACE (4 * IR_PULSE_TIME) // Space duration for logic 1
#define IR_MESSAGE_BIT_MAX 32  // Maximum number of bits in a NEC message
#define IR_TIMING_SLACK_US 200 // Tolerance around timing target
#define IR_BUFFER_SIZE 100     // Maximum number of events to store
#define IR_IN_TIMING_WINDOW(value, target, tolerance)                          \
  (target + tolerance > value && value > target - tolerance)

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

//
#endif
