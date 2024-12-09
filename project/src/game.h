#ifndef _GAME_H_
#define _GAME_H_

#include <pico.h>

typedef struct pong_rect {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint16_t color;
  uint16_t v_y;
  uint16_t v_x;
} pong_rect;

struct game_state {
  uint16_t bg_color;

  uint16_t padding_x;
  uint16_t padding_y;
  
  uint16_t canvas_w;
  uint16_t canvas_h;

  pong_rect Ball;
  pong_rect Player;
  pong_rect AI;
};




void update_game_state(struct game_state *gs);

#endif
