#ifndef _GAME_H_
#define _GAME_H_

#include <pico.h>

typedef struct pong_rect {
  uint16_t x;
  uint16_t y;

  uint16_t x_old;
  uint16_t y_old;

  uint16_t w;
  uint16_t h;

  uint16_t v_y;
  uint16_t v_x;

  uint16_t color;
} pong_rect;

struct game_state {
  uint16_t bg_color;

  uint16_t padding_x;
  uint16_t padding_y;

  uint16_t canvas_w;
  uint16_t canvas_h;

  pong_rect ball;
  pong_rect player;
  pong_rect ai;
  
  pong_rect ai_scoreboard;
  pong_rect player_scoreboard;

  pong_rect draw_point_player;
  pong_rect draw_point_ai;

  uint16_t player_score;
  uint16_t ai_score;
};

void gs_update_player(struct game_state *gs, int move_direction);
void gs_update_ball(struct game_state *gs);
void gs_update_ai(struct game_state *gs);
void gs_reset_ball(struct game_state *gs);


#endif
