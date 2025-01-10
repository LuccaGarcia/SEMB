#include "game.h"
#include <stdlib.h>

void update_paddle_position(pong_rect *paddle, int move_direction,
                            uint16_t padding_y, uint16_t canvas_h) {
  // Move the paddle based on the input direction
  paddle->y_old = paddle->y;
  paddle->y += move_direction;

  // Bounds checking to keep the paddle within the canvas
  if (paddle->y < padding_y) {
    paddle->y = padding_y; // Ensure paddle stays within upper bound
  } else if (paddle->y + paddle->h > canvas_h - padding_y) {
    paddle->y = canvas_h - paddle->h - padding_y; // Lower bound
  }
}

void gs_update_ball(struct game_state *gs) {
  // Update ball position
  gs->ball.x_old = gs->ball.x;
  gs->ball.y_old = gs->ball.y;

  gs->ball.x += gs->ball.v_x;
  gs->ball.y += gs->ball.v_y;

  // Check horizontal bounds
  if (gs->ball.x + gs->ball.w > gs->canvas_w - gs->padding_x) {
    gs_reset_ball(gs);
    gs->player_score++;
  } else if (gs->ball.x < gs->padding_x) {
    gs_reset_ball(gs);
    gs->ai_score++;
  }

  // Check vertical bounds
  if (gs->ball.y + gs->ball.h > gs->canvas_h - gs->padding_y) {
    gs->ball.y = gs->canvas_h - gs->ball.h - gs->padding_y;
    gs->ball.v_y *= -1; // Reverse velocity
  } else if (gs->ball.y < gs->padding_y) {
    gs->ball.y = gs->padding_y;
    gs->ball.v_y *= -1; // Reverse velocity
  }

  // Check collision with player paddle
  if (gs->ball.x < gs->player.x + gs->player.w &&
      gs->ball.x + gs->ball.w > gs->player.x &&
      gs->ball.y < gs->player.y + gs->player.h &&
      gs->ball.y + gs->ball.h > gs->player.y) {
    gs->ball.x = gs->player.x + gs->player.w; // Place ball at paddle edge
    gs->ball.v_x *= -1;                       // Reverse velocity
  }

  // Check collision with AI paddle
  if (gs->ball.x < gs->ai.x + gs->ai.w && gs->ball.x + gs->ball.w > gs->ai.x &&
      gs->ball.y < gs->ai.y + gs->ai.h && gs->ball.y + gs->ball.h > gs->ai.y) {
    gs->ball.x = gs->ai.x - gs->ball.w; // Place ball at paddle edge
    gs->ball.v_x *= -1;                 // Reverse velocity
  }
}

void gs_update_player(struct game_state *gs, int move_direction) {
  // Move the player paddle
  update_paddle_position(&gs->player, move_direction * gs->player.v_y,
                         gs->padding_y, gs->canvas_h);
}

void gs_update_ai(struct game_state *gs) {
  // Determine AI paddle movement
  int move_direction = 0;
  if (gs->ball.y < gs->ai.y) {
    move_direction = -1; // Move paddle up
  } else if (gs->ball.y > gs->ai.y + gs->ai.h) {
    move_direction = 1; // Move paddle up; // Move paddle down
  }

  // Move the AI paddle
  update_paddle_position(&gs->ai, move_direction * gs->ai.v_y, gs->padding_y,
                         gs->canvas_h);
}

// function that calculates if the ball is going to hit the left or right wall
// and resets the game and the ball spwaning at the center
void gs_reset_ball(struct game_state *gs) {

  // random y offset for the ball
  int16_t y_offset = (rand() % 50) - 25;

  // random ball direction for the x and y axis
  // x_dir = 1 or -1
  // y_dir = 1 or -1
  int16_t x_dir = (rand() % 2) ? 1 : -1;
  int16_t y_dir = (rand() % 2) ? 1 : -1;

  gs->ball.x = gs->canvas_w / 2;
  gs->ball.y = gs->canvas_h / 2 + y_offset;
  gs->ball.v_x = 2 * x_dir;
  gs->ball.v_y = 2 * y_dir;
}
