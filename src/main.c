#include <stdio.h>

#include <pico/stdio.h>
#include <pico/time.h>

int main() {
  stdio_init_all();
  // For more examples of UART use see
  // https://github.com/raspberrypi/pico-examples/tree/master/uart

  while (true) {
    printf("Hello, world!\n");
    sleep_ms(1000);
  }
}
