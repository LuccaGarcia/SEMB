#ifndef _GAME_H_
#define _GAME_H_

#include <pico.h>

struct game_state {
  uint16_t bg_color;
  uint16_t rect_color;

  uint16_t padding_x;
  uint16_t padding_y;

  uint16_t rect_x;
  uint16_t rect_y;
  uint16_t rect_w;
  uint16_t rect_h;

  int16_t v_x;
  int16_t v_y;

  uint16_t canvas_w;
  uint16_t canvas_h;
};

void update_game_state(struct game_state *gs);

#endif
