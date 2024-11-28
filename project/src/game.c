#include "game.h"

void update_game_state(struct game_state *gs) {
  // Check horizontal bounds
  if (gs->rect_x + gs->rect_w > gs->canvas_w - gs->padding_x) {
    gs->rect_x = gs->canvas_w - gs->rect_w - gs->padding_x; // Correct position
    gs->v_x *= -1;                                          // Reverse velocity
  } else if (gs->rect_x < gs->padding_x) {
    gs->rect_x = gs->padding_x; // Correct position
    gs->v_x *= -1;              // Reverse velocity
  }

  // Check vertical bounds
  if (gs->rect_y + gs->rect_h > gs->canvas_h - gs->padding_y) {
    gs->rect_y = gs->canvas_h - gs->rect_h - gs->padding_y; // Correct position
    gs->v_y *= -1;                                          // Reverse velocity
  } else if (gs->rect_y < gs->padding_y) {
    gs->rect_y = gs->padding_y; // Correct position
    gs->v_y *= -1;              // Reverse velocity
  }

  // Update position
  gs->rect_x += gs->v_x;
  gs->rect_y += gs->v_y;
}
