
#define TB_IMPL
#include "draw.h"
#include <stdio.h>

static int selectedStyle = FANCY;

static short drawWalls = 0;
static short drawScore = 0;
static Point tailLastPoint = {-1, -1};
static Point foodLastPoint = {-1, -1};
static int minX_, minY_, maxX_, maxY_;
static int snakeSize = 0;

int init_scr(int selStyle, int *maxX, int *maxY, short arcadeMode,
             short dScore) {

  selectedStyle = selStyle;
  drawScore = dScore;
  maxX_ = *maxX;
  maxY_ = *maxY;
  minX_ = 0;
  minY_ = 0;

  if (tb_init() != TB_OK) {
    printf("Failed to initialize termbox\n");
    return 0;
  }

  int cols = tb_width();
  int rows = tb_height();

  if (maxX_ || maxY_)
    drawWalls = 1;
  // The width of the game board is half of the columns because I use two
  // characters to represent one point ("██" or "▀ ")
  if (maxX_ == 0)
    maxX_ = cols / 2;
  else
    minX_ = (cols - maxX_ * 2) / 2;
  if (maxY_ == 0)
    maxY_ = rows - drawScore * 2;
  else
    minY_ = (rows - maxY_) / 2;

  if (arcadeMode) {
    minX_ = (cols - 60) / 2;
    minY_ = (rows - 24) / 2;
    maxX_ = 30;
    maxY_ = 22;
    drawScore = 1;
    drawWalls = 1;
  }

  if (cols < maxX_ * 2 || rows < maxY_) {
    tb_shutdown();
    if (arcadeMode)
      printf("The arcade mode requires a minimum of 64 columns by 22 rows\n");
    else
      printf("Terminal is too small for the inputted dimensions\n");
    return 0;
  }

  tb_hide_cursor();

  *maxX = maxX_;
  *maxY = maxY_;

  return 1;
}

// this funtion only draws the head and deletes the tail
// the rest of the body is not redraw unless the fancy or ascii mode are active
// in that case the head and the second section of the body are draw
void draw_snake(Snake *snake) {
  SnakePart *sPart = snake->head;
  SnakePart *sPart2 = snake->tail;

  // deletes last point where the tail was
  if (tailLastPoint.x != sPart2->x || tailLastPoint.y != sPart2->y) {

    if (tailLastPoint.x != -1 && tailLastPoint.y != -1)
      draw_point(tailLastPoint.x, tailLastPoint.y, 0, 7);
    tailLastPoint.x = sPart2->x;
    tailLastPoint.y = sPart2->y;
  }

  switch (selectedStyle) {
  case ASCII:
    // draw the head
    draw_point(sPart->x, sPart->y, 0, 9);
    // draw the second section of the body
    draw_point(sPart->next->x, sPart->next->y, 0, 8);
    // draw tail
    draw_point(sPart2->x, sPart2->y, 0, 18);
    break;
  case FANCY:
    // draw the head
    if (sPart->next->x == sPart->x + 1 && sPart->next->y == sPart->y) {
      draw_point(sPart->x, sPart->y, 0, 2);
    } else if (sPart->next->x == sPart->x - 1 && sPart->next->y == sPart->y) {

      draw_point(sPart->x, sPart->y, 0, 1);
    } else if (sPart->next->x == sPart->x && sPart->next->y == sPart->y + 1) {
      draw_point(sPart->x, sPart->y, 0, 3);
    } else if (sPart->next->x == sPart->x && sPart->next->y == sPart->y - 1) {

      draw_point(sPart->x, sPart->y, 0, 1);
    }

    if (snake->teleport) {
      if ((sPart->next->x == 0 && sPart->x == maxX_ - 1 &&
           sPart->next->y == sPart->y) ||
          (sPart->next->x == maxX_ - 1 && sPart->x == 0 &&
           sPart->next->y == sPart->y) ||
          (sPart->next->x == sPart->x && sPart->next->y == 0 &&
           sPart->y == maxY_ - 1) ||
          (sPart->next->x == sPart->x && sPart->next->y == maxY_ - 1 &&
           sPart->y == 0)) {
        draw_point(sPart->x, sPart->y, 0, 1);
      }
    }

    // draw the tail
    if (sPart2->prev->x == sPart2->x + 1 && sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, 0, 2);
    } else if (sPart2->prev->x == sPart2->x - 1 &&
               sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, 0, 1);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y + 1) {
      draw_point(sPart2->x, sPart2->y, 0, 3);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y - 1) {
      draw_point(sPart2->x, sPart2->y, 0, 1);
    }
    // draw the second section of the body
    if (snake->length > 2) {
      SnakePart *sPart3 = sPart->next;
      if (sPart3->prev->x == sPart3->x + 1 && sPart3->prev->y == sPart3->y &&
          sPart3->next->x == sPart3->x && sPart3->next->y == sPart3->y + 1) {

        draw_point(sPart3->x, sPart3->y, 0, 4);
      } else if (sPart3->prev->x == sPart3->x &&
                 sPart3->prev->y == sPart3->y + 1 &&
                 sPart3->next->x == sPart3->x + 1 &&
                 sPart3->next->y == sPart3->y) {

        draw_point(sPart3->x, sPart3->y, 0, 4);
      } else if (sPart3->prev->x == sPart3->x + 1 &&
                 sPart3->prev->y == sPart3->y) {
        draw_point(sPart3->x, sPart3->y, 0, 2);
      } else if (sPart3->prev->x == sPart3->x &&
                 sPart3->prev->y == sPart3->y + 1) {
        draw_point(sPart3->x, sPart3->y, 0, 3);
      } else if (sPart3->next->x == sPart3->x + 1 &&
                 sPart3->next->y == sPart3->y) {

        draw_point(sPart3->x, sPart3->y, 0, 2);
      } else if (sPart3->next->x == sPart3->x &&
                 sPart3->next->y == sPart3->y + 1) {
        draw_point(sPart3->x, sPart3->y, 0, 3);
      } else {
        draw_point(sPart3->x, sPart3->y, 0, 1);
      }
    }
    break;
  case FULL:
    // Draw the head
    draw_point(sPart->x, sPart->y, 0, 0);
    // The tail is drawn like a half block to avoid the Ouroboros.
    if (sPart2->prev->x == sPart2->x + 1 && sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, 0, 6);
    } else if (sPart2->prev->x == sPart2->x - 1 &&
               sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, 0, 3);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y + 1) {
      draw_point(sPart2->x, sPart2->y, 0, 5);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y - 1) {
      draw_point(sPart2->x, sPart2->y, 0, 2);
    }
    break;
  case DOTS:
    // draw the head
    draw_point(sPart->x, sPart->y, 5, 1);
    // draw the second section of the body
    draw_point(sPart->next->x, sPart->next->y, 0, 1);
    break;
  }
}
void draw_food(Point food) {

  if ((foodLastPoint.x != food.x || foodLastPoint.y != food.y) && food.x >= 0) {
    switch (selectedStyle) {
    case ASCII:
      draw_point(food.x, food.y, 2, 11);
      break;
    case DOTS:
    case FANCY:
      draw_point(food.x, food.y, 2, 1);
      break;
    case FULL:
      draw_point(food.x, food.y, 2, 0);
      break;
    }

    foodLastPoint.x = food.x;
    foodLastPoint.y = food.y;
  }
}
void draw_junk(List *junkList) {
  for (Node *it = junkList->first; it != NULL; it = it->next) {
    Point *jpoint = (Point *)it->data;
    switch (selectedStyle) {

    case ASCII:
      draw_point(jpoint->x, jpoint->y, 3, 10);
      break;
    case DOTS:
    case FANCY:
      draw_point(jpoint->x, jpoint->y, 3, 1);
      break;
    case FULL:

      draw_point(jpoint->x, jpoint->y, 3, 0);
      break;
    }
  }
}

/*
  All the snake games in the terminal that I found use ascii characters
  and lets be honest, they are kinda ugly.

  So I play around using unicode blocks:

  To get the spacing in the fancy mode we use this two unicode characters
  ▀ and █.
  A point/SnakePart is represented in two characters.
  We have to fill the gaps to connect the body parts
  You may understand it better like this:

  length    | not gap filled  | gap filled
  ----------|-----------------|-----------
  1         | ▀               | ▀
  2         | ▀ ▀             | ▀▀▀
  3         | ▀ ▀ ▀           | ▀▀▀▀▀

  so from this two characters ▀ and █ we have this two extra combination to fill
  the gaps in the snake body ▀▀ and █▀

  And like this we can have a really cool snake.

  ▀▀▀▀█
  █▀▀▀▀ █
  ▀▀▀▀▀▀▀
  This is the "sexy" part in the name sssnake!!!.

*/

void draw_point(int x, int y, short color, int type) {
  int ty = y + minY_;
  int tx = 2 * x + minX_;
  if (tx >= tb_width() || tx < 0 || ty >= tb_height() || ty < 0)
    return;

  uintattr_t fg;
  switch (color) {
  case 2:  fg = TB_GREEN;   break;
  case 3:  fg = TB_RED;     break;
  case 5:  fg = TB_YELLOW;  break;
  default: fg = TB_DEFAULT; break;
  }
  uintattr_t bg = TB_DEFAULT;

  uint32_t ch1, ch2;
  int bold = 0;

  switch (type) {
  case 0:  ch1 = 0x2588; ch2 = 0x2588; break; /* ██ */
  case 1:  ch1 = 0x2580; ch2 = ' ';    break; /* ▀  */
  case 2:  ch1 = 0x2580; ch2 = 0x2580; break; /* ▀▀ */
  case 3:  ch1 = 0x2588; ch2 = ' ';    break; /* █  */
  case 4:  ch1 = 0x2588; ch2 = 0x2580; break; /* █▀ */
  case 5:  ch1 = 0x2584; ch2 = 0x2584; break; /* ▄▄ */
  case 6:  ch1 = ' ';    ch2 = 0x2588; break; /*  █ */
  case 7:  ch1 = ' ';    ch2 = ' ';    break; /* erase */
  case 8:  ch1 = 'o';    ch2 = ' ';    bold = 1; break;
  case 9:  ch1 = '@';    ch2 = ' ';    bold = 1; break;
  case 10: ch1 = 'x';    ch2 = ' ';    bold = 1; break;
  case 11: ch1 = '8';    ch2 = ' ';    bold = 1; break;
  case 12: ch1 = '-';    ch2 = '-';    break;
  case 13: ch1 = '|';    ch2 = ' ';    break;
  case 14: ch1 = ' ';    ch2 = '|';    break;
  case 16: ch1 = 0x259A; ch2 = ' ';    break; /* ▚  */
  case 17: ch1 = 0x259A; ch2 = 0x2580; break; /* ▚▀ */
  case 18: ch1 = '.';    ch2 = ' ';    bold = 1; break;
  default: return;
  }

  if (bold)
    fg |= TB_BOLD;

  tb_set_cell(tx, ty, ch1, fg, bg);
  if (tx + 1 < tb_width())
    tb_set_cell(tx + 1, ty, ch2, fg, bg);
}

void draw_score(Snake *snake) {
  if (snakeSize != snake->length) {
    snakeSize = snake->length;
    char buf[32];
    snprintf(buf, sizeof(buf), "Size %i ", snake->length);
    tb_print(minX_, minY_ + maxY_ + 1, TB_DEFAULT, TB_DEFAULT, buf);
  }
}

void draw_walls() {
  switch (selectedStyle) {
  case DOTS:
  case FULL:
  case FANCY:
    if (drawScore || drawWalls) {
      for (int i = 0; i < maxX_; i++)
        draw_point(i, maxY_, 4, 2);
    }
    if (drawWalls) {
      for (int i = -1; i < maxX_ + 1; i++) {
        draw_point(i, -1, 4, 2);
      }

      for (int i = 0; i < maxY_; i++) {
        draw_point(-1, i, 4, 16);
      }

      for (int i = -1; i < maxY_ + 1; i++) {
        draw_point(maxX_, i, 4, 16);
      }

      draw_point(-1, -1, 4, 17);
      draw_point(-1, maxY_, 4, 17);
    }
    break;
  case ASCII:
    if (drawScore || drawWalls) {
      for (int i = 0; i < maxX_; i++)
        draw_point(i, maxY_, 4, 12);
    }
    if (drawWalls) {
      for (int i = -1; i < maxX_ + 1; i++) {
        draw_point(i, -1, 4, 12);
      }

      for (int i = -1; i < maxY_ + 1; i++) {
        draw_point(-1, i, 4, 14);
      }

      for (int i = -1; i < maxY_ + 1; i++) {
        draw_point(maxX_, i, 4, 13);
      }
    }
    break;
  }
}
