#include "game.h"

void update_game_state(struct game_state *gs) {
  // Check horizontal bounds

  if (gs->Ball.x + gs->Ball.w > gs->canvas_w - gs->padding_x) {
    gs->Ball.x = gs->canvas_w - gs->Ball.w - gs->padding_x; // Correct position
    gs->Ball.v_x *= -1;                                     // Reverse velocity
  } else if (gs->Ball.x < gs->padding_x) {
    gs->Ball.x = gs->padding_x; // Correct position
    gs->Ball.v_x *= -1;         // Reverse velocity
  }

  if (gs->Ball.y + gs->Ball.h > gs->canvas_h - gs->padding_y) {
    gs->Ball.y = gs->canvas_h - gs->Ball.h - gs->padding_y; // Correct position
    gs->Ball.v_y *= -1;                                     // Reverse velocity
  } else if (gs->Ball.y < gs->padding_y) {
    gs->Ball.y = gs->padding_y; // Correct position
    gs->Ball.v_y *= -1;         // Reverse velocity
  }
  // Update position

  gs->Ball.x += gs->Ball.v_x;
  gs->Ball.y += gs->Ball.v_y;

  // bounces the ball back if it hits the player
  if (gs->Ball.x < gs->Player.x + gs->Player.w && gs->Ball.x + gs->Ball.w > gs->Player.x && gs->Ball.y < gs->Player.y + gs->Player.h && gs->Ball.y + gs->Ball.h > gs->Player.y) {
    gs->Ball.x = gs->Player.x + gs->Player.w;
    gs->Ball.v_x *= -1;
  }

  // bounces the ball back if it hits the AI
  if (gs->Ball.x < gs->AI.x + gs->AI.w && gs->Ball.x + gs->Ball.w > gs->AI.x && gs->Ball.y < gs->AI.y + gs->AI.h && gs->Ball.y + gs->Ball.h > gs->AI.y) {
    gs->Ball.x = gs->AI.x - gs->Ball.w;
    gs->Ball.v_x *= -1;
  }

  


  //! TODO implement player colision with ball
  //! TODO implement AI colision with ball
  //! TODO implement AI movement
  //! TODO implement player movement


}
