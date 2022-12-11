
#include "draw.h"

static int selectedStyle = FANCY;

static short foodColor = TB_GREEN;
static short snakeColor = TB_DEFAULT;
static short snakeHeadColor = TB_YELLOW;
static short junkColor = TB_RED;
static short wallColor = TB_DEFAULT;
static short drawWalls = 0;
static short drawScore = 0;
static Point tailLastPoint = {-1, -1};
static Point foodLastPoint = {-1, -1};
static int minX_, minY_, maxX_, maxY_, rows, cols;
static int snakeSize = 0;

int init_scr(int selStyle, int *maxX, int *maxY, short arcadeMode,
             short dScore) {

  selectedStyle = selStyle;
  drawScore = dScore;
  maxX_ = *maxX;
  maxY_ = *maxY;
  minX_ = 0;
  minY_ = 0;

  tb_init();

  tb_hide_cursor();
  // setlocale(LC_ALL, "");
  // cbreak();
  // initscr();
  // if (has_colors() == FALSE) {
  //   endwin();
  //   printf("Your terminal does not support color\n");
  //   return 0;
  // }
  // getmaxyx(stdscr, rows, cols);

  cols = tb_width();
  rows = tb_height();

  cols = cols % 2 == 0 ? cols : cols + 1;
  rows = rows % 2 == 0 ? rows : rows + 1;

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
  // maxX = 10;
  // maxY = 10;
  if (arcadeMode) {

    minX_ = (cols - 60) / 2;
    minY_ = (rows - 24) / 2;
    maxX_ = 30;
    maxY_ = 22;
    drawScore = 1;
    drawWalls = 1;
  } // else if (selectedMode == SCREENSAVER || selectedMode == AUTOPILOT)
  // teleport = 0;

  if (cols < maxX_ * 2 || rows < maxY_) {

    tb_shutdown();
    if (arcadeMode)
      printf("The arcade mode requires a minimum of 64 colums by 22 rows\n");
    else
      printf("Terminal is too small for the required dimensions");
    return 0;
  }

  // noecho();
  // keypad(stdscr, TRUE);
  // start_color();
  // use_default_colors();
  // init_pair(1, snakeColor, -1);
  // init_pair(2, foodColor, -1);
  // init_pair(3, junkColor, -1);
  // init_pair(4, wallColor, -1);
  // init_pair(5, snakeHeadColor, -1);
  // curs_set(0);

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
    draw_point(sPart->x, sPart->y, snakeColor, 9);
    // draw the second section of the body
    draw_point(sPart->next->x, sPart->next->y, snakeColor, 8);
    // draw tail
    draw_point(sPart2->x, sPart2->y, snakeColor, 18);
    break;
  case FANCY:
    // draw the head
    if (sPart->next->x == sPart->x + 1 && sPart->next->y == sPart->y) {
      draw_point(sPart->x, sPart->y, snakeColor, 2);
    } else if (sPart->next->x == sPart->x - 1 && sPart->next->y == sPart->y) {

      draw_point(sPart->x, sPart->y, snakeColor, 1);
    } else if (sPart->next->x == sPart->x && sPart->next->y == sPart->y + 1) {
      draw_point(sPart->x, sPart->y, snakeColor, 3);
    } else if (sPart->next->x == sPart->x && sPart->next->y == sPart->y - 1) {

      draw_point(sPart->x, sPart->y, snakeColor, 1);
    }

    if (snake->teleport) {
      if (sPart->next->x == 0 && sPart->x == maxX_ - 1 &&
          sPart->next->y == sPart->y) {

        // draw_point(sPart->x + 1, sPart->y, snakeColor, 19);
        draw_point(sPart->x, sPart->y, snakeColor, 1);
      } else if (sPart->next->x == maxX_ - 1 && sPart->x == 0 &&
                 sPart->next->y == sPart->y) {

        // draw_point(sPart->next->x + 1, sPart->next->y, snakeColor, 19);
        draw_point(sPart->x, sPart->y, snakeColor, 1);
      } else if (sPart->next->x == sPart->x && sPart->next->y == 0 &&
                 sPart->y == maxY_ - 1) {
        // draw_point(sPart->x, sPart->y + 1, snakeColor, 19);
        draw_point(sPart->x, sPart->y, snakeColor, 1);
      } else if (sPart->next->x == sPart->x && sPart->next->y == maxY_ - 1 &&
                 sPart->y == 0) {
        // draw_point(sPart->next->x, sPart->next->y + 1, snakeColor, 19);
        draw_point(sPart->x, sPart->y, snakeColor, 1);
      }
    }

    // draw the tail
    if (sPart2->prev->x == sPart2->x + 1 && sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 2);
    } else if (sPart2->prev->x == sPart2->x - 1 &&
               sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 1);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y + 1) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 3);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y - 1) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 1);
    }
    // draw the second section of the body
    if (snake->length > 2) {
      SnakePart *sPart3 = sPart->next;
      if (sPart3->prev->x == sPart3->x + 1 && sPart3->prev->y == sPart3->y &&
          sPart3->next->x == sPart3->x && sPart3->next->y == sPart3->y + 1) {

        draw_point(sPart3->x, sPart3->y, snakeColor, 4);
      } else if (sPart3->prev->x == sPart3->x &&
                 sPart3->prev->y == sPart3->y + 1 &&
                 sPart3->next->x == sPart3->x + 1 &&
                 sPart3->next->y == sPart3->y) {

        draw_point(sPart3->x, sPart3->y, snakeColor, 4);
      } else if (sPart3->prev->x == sPart3->x + 1 &&
                 sPart3->prev->y == sPart3->y) {
        draw_point(sPart3->x, sPart3->y, snakeColor, 2);
      } else if (sPart3->prev->x == sPart3->x &&
                 sPart3->prev->y == sPart3->y + 1) {
        draw_point(sPart3->x, sPart3->y, snakeColor, 3);
      } else if (sPart3->next->x == sPart3->x + 1 &&
                 sPart3->next->y == sPart3->y) {

        draw_point(sPart3->x, sPart3->y, snakeColor, 2);
      } else if (sPart3->next->x == sPart3->x &&
                 sPart3->next->y == sPart3->y + 1) {
        draw_point(sPart3->x, sPart3->y, snakeColor, 3);
      } else {
        draw_point(sPart3->x, sPart3->y, snakeColor, 1);
      }
    }
    break;
  case FULL:
    // Draw the head
    draw_point(sPart->x, sPart->y, snakeColor, 0);
    // The tail is drawn like a half block to avoid the Ouroboros.
    if (sPart2->prev->x == sPart2->x + 1 && sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 6);
    } else if (sPart2->prev->x == sPart2->x - 1 &&
               sPart2->prev->y == sPart2->y) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 3);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y + 1) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 5);
    } else if (sPart2->prev->x == sPart2->x &&
               sPart2->prev->y == sPart2->y - 1) {
      draw_point(sPart2->x, sPart2->y, snakeColor, 2);
    }
    break;
  case DOTS:
    // draw the head
    draw_point(sPart->x, sPart->y, snakeHeadColor, 1);
    // draw the second section of the body
    draw_point(sPart->next->x, sPart->next->y, snakeColor, 1);
    break;
  }
}
void draw_food(Point food) {

  if ((foodLastPoint.x != food.x || foodLastPoint.y != food.y) && food.x >= 0) {
    switch (selectedStyle) {
    case ASCII:
      draw_point(food.x, food.y, foodColor, 11);
      break;
    case DOTS:
    case FANCY:
      draw_point(food.x, food.y, foodColor, 1);
      break;
    case FULL:
      draw_point(food.x, food.y, foodColor, 0);
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
      draw_point(jpoint->x, jpoint->y, junkColor, 10);
      break;
    case DOTS:
    case FANCY:
      draw_point(jpoint->x, jpoint->y, junkColor, 1);
      break;
    case FULL:

      draw_point(jpoint->x, jpoint->y, junkColor, 0);
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
  if (tx > cols || tx < 0 || ty > rows || ty < 0)
    return;
  // move(ty, tx);
  // attron(COLOR_PAIR(color));
  switch (type) {
  case 0:
    tb_printf(tx, ty, color, 0, "██");
    break;
  case 1:
    tb_printf(tx, ty, color, 0, "▀ ");
    break;
  case 2:
    tb_printf(tx, ty, color, 0, "▀▀");
    break;
  case 3:
    tb_printf(tx, ty, color, 0, "█ ");
    break;
  case 4:
    tb_printf(tx, ty, color, 0, "█▀");
    break;
  case 5:
    tb_printf(tx, ty, color, 0, "▄▄");
    break;
  case 6:
    tb_printf(tx, ty, color, 0, " █");
    break;
  case 7:
    tb_printf(tx, ty, color, 0, "  ");
    break;
  case 8:
    tb_printf(tx, ty, color, 0, "o ");
    break;
  case 9:
    tb_printf(tx, ty, color, 0, "@ ");
    break;
  case 10:
    tb_printf(tx, ty, color, 0, "x ");
    break;
  case 11:
    tb_printf(tx, ty, color, 0, "8 ");
    break;
  case 12:
    tb_printf(tx, ty, color, 0, "--");
    break;
  case 13:
    tb_printf(tx, ty, color, 0, "| ");
    break;
  case 14:
    tb_printf(tx, ty, color, 0, " |");
    break;
  case 16:
    tb_printf(tx, ty, color, 0, "▚ ");
    break;
  case 17:
    tb_printf(tx, ty, color, 0, "▚▀");
    break;
  case 18:
    tb_printf(tx, ty, color, 0, ". ");
    break;
  case 19:
    tb_printf(tx, ty, color, 0, "▀");
    break;
  }
  // move(rows, cols);
}

void draw_score(Snake *snake) {

  // move(minY_ + maxY_ + 1, minX_);

  // attron(COLOR_PAIR(4));
  if (snakeSize != snake->length) {
    snakeSize = snake->length;

    tb_printf(minX_, minY_ + maxY_ + 1, wallColor, 0, "Size %i ",
              snake->length);
  }
  // attroff(COLOR_PAIR(4));
}

void draw_walls() {
  switch (selectedStyle) {
  case DOTS:
  case FULL:
  case FANCY:
    if (drawScore || drawWalls) {
      for (int i = 0; i < maxX_; i++)
        draw_point(i, maxY_, wallColor, 2);
    }
    if (drawWalls) {
      for (int i = -1; i < maxX_ + 1; i++) {
        draw_point(i, -1, wallColor, 2);
      }

      for (int i = 0; i < maxY_; i++) {
        draw_point(-1, i, wallColor, 16);
      }

      for (int i = -1; i < maxY_ + 1; i++) {
        draw_point(maxX_, i, wallColor, 16);
      }

      draw_point(-1, -1, 4, 17);
      draw_point(-1, maxY_, wallColor, 17);
    }
    break;
  case ASCII:
    if (drawScore || drawWalls) {
      for (int i = 0; i < maxX_; i++)
        draw_point(i, maxY_, wallColor, 12);
    }
    if (drawWalls) {
      for (int i = -1; i < maxX_ + 1; i++) {
        draw_point(i, -1, wallColor, 12);
      }

      for (int i = -1; i < maxY_ + 1; i++) {
        draw_point(-1, i, wallColor, 14);
      }

      for (int i = -1; i < maxY_ + 1; i++) {
        draw_point(maxX_, i, wallColor, 13);
      }
    }
    break;
  }
}
