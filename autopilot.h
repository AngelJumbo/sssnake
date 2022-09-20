#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "essentials.h"
#include "snake.h"
#include "xymap.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum algorithm { BASIC, GREEDY1, GREEDY2 };

typedef struct cell {
  int parent_i, parent_j;

  double f, g, h;
  SnakePart *head;
} Cell;
Cell *cell_create(int parent_i, int parent_j, double f, double g, double h);

int is_valid(int x, int y, int maxX, int maxY);

int is_destination(int x, int y, Point dest);

int calculate_h_value(int x, int y, Point dest, int maxX, int maxY,
                      short teleport);

Stack *trace_path(Cell **cellDetails, Point dest, int maxX);

Stack *a_star_search(XYMap *map, Snake *snake, int maxX, int maxY, Point dest,
                     short checkBody);

Stack *try_hard(XYMap *xymap, Snake *snake, int maxX, int maxY, Point dest,
                short mode);

int long_step(XYMap *xymap, Stack *path, short teleport);
#endif // !AUTOPILOT_H
