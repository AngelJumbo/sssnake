#include <curses.h>
#include <getopt.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#include "autopilot.h"
#include "essentials.h"
#include "snake.h"
#include "xymap.h"

int direction = East;
int cols, rows;
int maxX, maxY;
int c = East;
short foodColor = COLOR_GREEN;
short snakeColor = -1;
short wallColor = COLOR_RED;

int autopilot = 0;
int fancy = 0;
int ascii = 0;
int screensaver = 0;
int speed = 200000;
int junk = 0;

XYMap *blocksTaken = NULL;
Snake *snake = NULL;
Stack *path = NULL;
List *junkList = NULL;

Point food;
Point foodLastPoint;
Point tailLastPoint;

void init_options(int argc, char *argv[]);
void init_game(void);
void draw_snake(Snake *snake);
void draw_food(int x, int y);
void draw_junk(List *junkList);
void draw_point(int x, int y, short color, int type);
int get_direction(int c);
void print_help();

int main(int argc, char *argv[]) {

  init_options(argc, argv);
  init_game();

  curs_set(0);

  // The width of the game board is half of the columns because I use two
  // characters to represent one point ("██" or "▀ ")
  maxX = cols / 2;
  maxY = rows;

  do { // This loop goes forever if the option "screensaver" is given.

    // Used to know where the body of the snake and the junk are,
    // blocksTaken is just a matrix of 2x2 but It can change its dimesions
    // dinamically if I ever decide to change  them by detecting changes in the
    // dimensions of the terminal.
    blocksTaken = xymap_create(maxX, maxY);
    snake = snake_create(maxX / 2, maxY / 2, direction);
    // A snake with a size of just one cell may break somethings
    // so just grow it by one at the start.
    snake->grow = 1;

    // Used to know when a new path is needed in autopilot and screensaver mode
    // If the food does not change position then just follow the actual path.
    foodLastPoint.x = -1;
    foodLastPoint.y = -1;
    // Used to know what block should be erased from the screen when the snake
    // moves because the program does not redraws everything in every game loop
    // it just draws a new point for the head and deletes the last point of the
    // tail.
    tailLastPoint.x = -1;
    tailLastPoint.y = -1;

    // Self explainatory block (I hope)
    if (junk) {
      junkList = list_create();
      int maxJunk = (maxX * maxY) * junk / 100;
      for (int i = 0; i < maxJunk; i++) {
        int jx, jy;
        do {

          jx = rand() % maxX;
          jy = rand() % maxY;
        } while (
            xymap_marked(blocksTaken, jx, jy) ||
            (jy == maxY / 2 &&
             (jx > maxX / 4 && jy < maxX * 3 / 4))); // avoid snake spawn blocks
        xymap_mark(blocksTaken, jx, jy, WALL);
        list_append(junkList, point_create(jx, jy));
      }

      draw_junk(junkList);
    }

    rand_pos_food(&food, blocksTaken, maxX, maxY);

    // The game loop
    while (1) {
      // Get a new path to follow if the autopilot or the screensaver are active
      if ((autopilot) &&
          (foodLastPoint.x != food.x || foodLastPoint.y != food.y)) {
        Point src;
        // TODO: the source point is the head of the snake, this parameter is
        // redundant.
        src.x = snake->head->x;
        src.y = snake->head->y;
        if (path != NULL) {
          stack_free(path);
        }

        path = a_star_search(blocksTaken, snake, maxX, maxY, src, food);
        if (path != NULL) {

          free(stack_pop(path));

        } else if (snake->length > 1) {
          // if there is no path found then mark the direction to which the
          // snake was headed and can follow it to die
          if ((snake->head->x - (snake->head->next->x + 1)) == 0)
            direction = East;
          if ((snake->head->x - (snake->head->next->x - 1)) == 0)
            direction = West;
          if ((snake->head->y - (snake->head->next->y + 1)) == 0)
            direction = South;
          if ((snake->head->y - (snake->head->next->y - 1)) == 0)
            direction = North;
        }
        foodLastPoint.x = food.x;
        foodLastPoint.y = food.y;
      }

      timeout(0);
      c = getch();
      // move(rows, cols);

      if (path != NULL) {

        Point *point = stack_pop(path);
        update_position_autopilot(snake, blocksTaken, &food, point->x, point->y,
                                  maxX, maxY);
        free(point);
      } else {
        // If the autopilot is on and there is no path to follow then let the
        // snake die.
        if (!autopilot)
          direction = get_direction(c);
        update_position(snake, blocksTaken, &food, direction, maxX, maxY);
      }

      // ignore this, is just a debug block
      // xymap_print_log(blocksTaken, snake->head->x, snake->head->y,
      //                 snake->tail->x, snake->tail->y);

      // FILE *fp;
      // fp = fopen("log.txt", "a+");

      // fprintf(fp, "Food (%i,%i) \n", food.x, food.y);
      // fclose(fp);

      if (snake->head->x == food.x && snake->head->y == food.y)
        rand_pos_food(&food, blocksTaken, maxX, maxY);

      if (snake->colission || c == 'q')
        break;
      if (screensaver && c != ERR) {
        c = 'q';
        break;
      }
      draw_snake(snake);
      draw_food(food.x, food.y);
      c = 0;
      refresh();
      usleep(speed);
    }

    clear();
    refresh();
    if (junk)
      list_free(junkList);
    snake_free(snake);
    xymap_free(blocksTaken);

    if (c == 'q')
      break;
  } while (screensaver);
  endwin();
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

  so from this two characters ▀ and █ we have this extra combination to fill the
  gaps in the snake body ▀▀ and █▀

  And like this we can have a really cool snake.

  ▀▀▀▀█
  █▀▀▀▀ █
  ▀▀▀▀▀▀▀
  This is the "sexy" part in the name sssnake!!!.

  The other cases 5 and 6 below were intended to build walls but for now are
  unused.
*/

void draw_point(int x, int y, short color, int type) {
  move(y, 2 * x);
  attron(COLOR_PAIR(color));
  switch (type) {
  case 0:
    addwstr(L"██");
    break;
  case 1:
    addwstr(L"▀ ");
    break;
  case 2:
    addwstr(L"▀▀");
    break;
  case 3:
    addwstr(L"█ ");
    break;
  case 4:
    addwstr(L"█▀");
    break;
  case 5:
    addwstr(L"▄▄");
    break;
  case 6:
    addwstr(L" █");
    break;
  case 7:
    addstr("  ");
    break;
  case 8:
    addstr("o ");
    break;
  case 9:
    addstr("@ ");
    break;
  case 10:
    addstr("x ");
    break;
  case 11:
    addstr("8 ");
    break;
  }

  attroff(COLOR_PAIR(color));
  // move(rows, cols);
}

int get_direction(int c) {
  switch (c) {
  case KEY_UP:
  case 'k':
  case 'K':
  case 'w':
  case 'W':
    return North;
    break;
  case KEY_DOWN:
  case 'j':
  case 'J':
  case 's':
  case 'S':
    return South;
    break;
  case KEY_LEFT:
  case 'a':
  case 'A':
  case 'h':
  case 'H':
    return West;
    break;
  case KEY_RIGHT:
  case 'd':
  case 'D':
  case 'l':
  case 'L':
    return East;
    break;
  default:
    return -1;
    break;
  }
  return -1;
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

  if (ascii) {
    // draw the head
    draw_point(sPart->x, sPart->y, 0, 9);
    // draw the second section of the body
    draw_point(sPart->next->x, sPart->next->y, 0, 8);

  } else if (fancy) {
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
    // draw the second section of the body
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

  } else {
    // Draw the head
    draw_point(sPart->x, sPart->y, 0, 0);
    // The tail is drawn like a half block to avoid the Ouroboros.
    // Then, why not give the head another color?
    // Because I don't like it, but it can be implemented later as an
    // alternative.
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
  }
}
void draw_food(int x, int y) {
  if (ascii) {

    draw_point(x, y, 2, 11);
  } else if (fancy) {
    draw_point(x, y, 2, 1);
  } else {

    draw_point(x, y, 2, 0);
  }
}
void draw_junk(List *junkList) {
  for (Node *it = junkList->first; it != NULL; it = it->next) {
    Point *jpoint = (Point *)it->data;
    if (ascii) {

      draw_point(jpoint->x, jpoint->y, 3, 10);
    } else

        if (fancy) {
      draw_point(jpoint->x, jpoint->y, 3, 1);
    } else {

      draw_point(jpoint->x, jpoint->y, 3, 0);
    }
  }
}
void init_game(void) {
  srand(time(NULL));

  setlocale(LC_ALL, "");
  initscr();
  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    return;
  }

  getmaxyx(stdscr, rows, cols);
  noecho();
  keypad(stdscr, TRUE);
  start_color();
  use_default_colors();
  init_pair(1, snakeColor, -1);
  init_pair(2, foodColor, -1);
  init_pair(3, wallColor, -1);
}

void init_options(int argc, char *argv[]) {
  int op;
  int speedMult;

  const struct option long_options[] = {
      {"help", 0, NULL, 'h'},        {"speed", 1, NULL, 's'},
      {"screensaver", 0, NULL, 'S'}, {"fancy", 0, NULL, 'f'},
      {"junk", 0, NULL, 'j'},        {"autopilot", 0, NULL, 'a'},
      {"ascii", 0, NULL, 'A'},       {NULL, 0, NULL, 0}};

  if (argc == 1) {
    printf("You ran this program with no extra options,\n"
           "maybe it was intentional but you might have \n"
           "more fun trying the available options, run:\n"
           "sssnake -h\n ");
  }

  while ((op = getopt_long(argc, argv, ":aSfs:j:hA", long_options, NULL)) !=
         -1) {
    switch (op) {
    case 'A':
      ascii = 1;
      break;
    case 'a':
      autopilot = 1;
      break;
    case 'S':
      // When this option is given the autopilot flag is true too
      autopilot = 1;
      screensaver = 1;
      break;
    case 'f':
      fancy = 1;
      break;
    case 'j':
      junk = atoi(optarg);
      if (junk <= 0 || junk > 5)
        junk = 0;
      break;
    case 's':
      speedMult = 21 - atoi(optarg);
      if (speedMult > 0 && speedMult <= 20) {
        speed = 10000 * speedMult;
      }
      break;
    case 'h':
      print_help();
      exit(0);
    case ':':
      printf("option needs a value\n");
      exit(0);
    case '?':
      printf("unknown option: %c\n", optopt);
      exit(0);
    }
  }
}

void print_help() {

  printf(
      "Usage: sssnake [OPTIONS]\n"
      "Options:\n"
      "  -a, --autopilot    The game plays itself. (Default: no)\n"
      "  -A, --ascii        Use ascii characters. (Default: no)\n"
      "  -s, --speed=N      Speed of the game, from 1 to 20. (Default: 1 )\n"
      "  -S, --screensaver  Autopilot, but it restarts when it dies. (Default: "
      "no)\n"
      "  -j, --junk=N       Add random blocks of junk, levels from 1 to 5. "
      "(Default: 0 )\n"
      "  -f, --fancy        Add a fancy spacing between blocks. (Default: no)\n"
      "  -h, --help         Print help message. \n"
      "Try to run something like this :\n"
      "sssnake -s 15 -f -j 5 -S\n"
      "Warning!!! be careful using the screensaver and junk options "
      "at the same time on OLED screens.\nIf the snake is to much time alive "
      "the static dots of junk may burn your screen.\n"
      "Using the screensaver option alone should be fine.\n"
      "For bugs or new features go to : "
      "https://github.com/AngelJumbo/sssnake\n");
}
