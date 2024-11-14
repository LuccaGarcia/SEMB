#include <stdio.h>

#include <pico/stdio.h>
#include <pico/time.h>

int main() {
  stdio_init_all();

  while (true) {
    printf("Hello, world!\n");
    sleep_ms(1000);
  }
}
