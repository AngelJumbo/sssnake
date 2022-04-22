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

void init_game(void);
void draw_snake(Snake *snake);
void draw_food(int x, int y);
void draw_junk(List *junkList);
void draw_point(int x, int y, short color, int type);
int get_direction(int c);
void print_help();

int main(int argc, char *argv[]) {
  int op;
  int speedMult;

  const struct option long_options[] = {
      {"help", 0, NULL, 'h'},        {"speed", 1, NULL, 's'},
      {"screensaver", 0, NULL, 'S'}, {"fancy", 0, NULL, 'f'},
      {"junk", 0, NULL, 'j'},        {"autopilot", 0, NULL, 'a'},
      {"ascii", 0, NULL, 'A'},       {NULL, 0, NULL, 0}};
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
      return 0;
    case ':':
      printf("option needs a value\n");
      return 0;
    case '?':
      printf("unknown option: %c\n", optopt);
      return 0;
    }
  }

  init_game();

  curs_set(0);
  maxX = cols / 2;
  maxY = rows;

  do {

    blocksTaken = xymap_create(maxX, maxY);
    snake = snake_create(maxX / 2, maxY / 2, direction);
    snake->grow = 1;
    foodLastPoint.x = -1;
    foodLastPoint.y = -1;
    tailLastPoint.x = -1;
    tailLastPoint.y = -1;

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
    while (1) {
      if ((autopilot) &&
          (foodLastPoint.x != food.x || foodLastPoint.y != food.y)) {
        Point src;
        Point dest;

        src.x = snake->head->x;
        src.y = snake->head->y;
        dest.x = food.x;
        dest.y = food.y;
        if (path != NULL) {
          stack_free(path);
        }

        path = a_star_search(blocksTaken, snake, maxX, maxY, src, dest);
        if (path != NULL) {

          free(stack_pop(path));

        } else if (snake->length > 1) {
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
      move(rows, cols);

      if (path != NULL) {

        Point *point = stack_pop(path);
        update_position_autopilot(snake, blocksTaken, &food, point->x, point->y,
                                  maxX, maxY);
        free(point);
      } else {
        if (!autopilot)
          direction = get_direction(c);
        update_position(snake, blocksTaken, &food, direction, maxX, maxY);
      }

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
    addstr("()");
    break;
  case 9:
    addstr("[]");
    break;
  case 10:
    addstr("XX");
    break;
  case 11:
    addstr("OO");
    break;
  }

  attroff(COLOR_PAIR(color));
  move(rows, cols);
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

void draw_snake(Snake *snake) {
  SnakePart *sPart = snake->head;
  SnakePart *sPart2 = snake->tail;

  if (tailLastPoint.x != sPart2->x || tailLastPoint.y != sPart2->y) {

    if (tailLastPoint.x != -1 && tailLastPoint.y != -1)
      draw_point(tailLastPoint.x, tailLastPoint.y, 0, 7);
    tailLastPoint.x = sPart2->x;
    tailLastPoint.y = sPart2->y;
  }

  if (ascii) {

    draw_point(sPart->x, sPart->y, 0, 9);

    draw_point(sPart->next->x, sPart->next->y, 0, 8);

  } else if (fancy) {

    if (sPart->next->x == sPart->x + 1 && sPart->next->y == sPart->y) {
      draw_point(sPart->x, sPart->y, 0, 2);
    } else if (sPart->next->x == sPart->x - 1 && sPart->next->y == sPart->y) {

      draw_point(sPart->x, sPart->y, 0, 1);
    } else if (sPart->next->x == sPart->x && sPart->next->y == sPart->y + 1) {
      draw_point(sPart->x, sPart->y, 0, 3);
    } else if (sPart->next->x == sPart->x && sPart->next->y == sPart->y - 1) {

      draw_point(sPart->x, sPart->y, 0, 1);
    }

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
void print_help() {

  printf(
      "Usage: sssnake [OPTIONS]\n"
      "Warning: if the characters are not displayed properly use the flag "
      "\"-A\" to run in ascii mode until I fix it."
      "Options:\n"
      "  -a, --autopilot    The game plays itself. (Default: no)\n"
      "  -A, --ascii        Use ascii characters. (Default: no)\n"
      "  -s, --speed=N      Speed of the game, from 1 to 20. (Default: 1 )\n"
      "  -S, --screensaver  Autopilot, but it restarts when it dies. (Default: "
      "no)\n"
      "  -j, --junk=N       Add random blocks of junk, levels from 1 to 5. "
      "(Default: 0 )\n"
      "  -f, --fancy        Add a fancy spacing between blocks. (Default: no)\n"
      "  -h, --help         Print help message. \n");
}
