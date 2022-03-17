#ifndef SNAKE_H
#define SNAKE_H

#include "xymap.h"
#include <stdlib.h>

enum direction { North, South, East, West };
typedef struct SnakePart_ {
  int x;
  int y;
  struct SnakePart_ *next;
  struct SnakePart_ *prev;
} SnakePart;

typedef struct Snake_ {
  SnakePart *head;
  SnakePart *tail;
  int direction;
  int length;
  int grow;
  int colission;
} Snake;

typedef struct point {
  int x;
  int y;
} Point;

SnakePart *snake_part_create(int x, int y, SnakePart *prev);
void snake_part_free(SnakePart *snakePart);

Snake *snake_create(int x, int y, int direction);
void snake_free(Snake *snake);

void update_position(Snake *snake, XYMap *blocksTaken, Point *food, int dir,
                     int maxX, int maxY);

void update_position_autopilot(Snake *snake, XYMap *blocksTaken, Point *food,
                               int x, int y, int maxX, int maxY);

void check_colission(Snake *sn, XYMap *blocksTaken, int x, int y, int maxX,
                     int maxY);

void rand_pos_food(Point *food, XYMap *blocksTaken, int maxX, int maxY);
Point *point_create(int x, int y);
#endif
