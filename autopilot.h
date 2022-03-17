#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "essentials.h"
#include "snake.h"
#include "xymap.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Most of this code is based on the code found in
// https://www.geeksforgeeks.org/a-search-algorithm/

typedef struct cell {
  int parent_i, parent_j;

  double f, g, h;
  SnakePart *head;
} Cell;
Cell *cell_create(int parent_i, int parent_j, double f, double g, double h);

// SnakePart *copy_and_move(SnakePart *head, int x, int y);

// A Utility Function to check whether given cell (y, x)
// is a valid cell or not.
int is_valid(int x, int y, int maxX, int maxY);

// A Utility Function to check whether the given cell is
// blocked or not
int is_unblocked(XYMap *xymap, Cell **cellDetails, int x, int y, int x2, int y2,
                 int maxX);
// A Utility Function to check whether destination cell has
// been reached or not
int is_destination(int x, int y, Point dest);
// A Utility Function to calculate the 'h' heuristics.
int calculate_h_value(int x, int y, Point dest);
// A Utility Function to trace the path from the source
// to destination
Stack *trace_path(Cell **cellDetails, Point dest, int maxX);
// A Function to find the shortest path between
// a given source cell to a destination cell according
// to A* Search Algorithm
Stack *a_star_search(XYMap *map, Snake *snake, int maxX, int maxY, Point src,
                     Point dest);
// Driver program to test above function
/*
int main() {
  XYMap *map = xymap_create(10);
  int maxX = 10;
  int maxY = 10;
  xymap_mark(map, 1, 1);
  xymap_mark(map, 1, 2);
  xymap_mark(map, 2, 2);
  xymap_mark(map, 2, 3);
  xymap_print(map);

  // Source is the left-most bottom-most corner
  Point *src = point_create(2, 3);

  // Destination is the left-most top-most corner
  Point *dest = point_create(5, 5);

  Stack *path = a_star_search(map, maxX, maxY, *src, *dest);
  if (path != NULL) {
    while (path->count > 0) {
      Point *point = (Point *)stack_pop(path);
      printf("(%i,%i)->", point->x, point->y);
    }
  } else {
    printf("Path invalid");
  }

  return (0);
}
*/
#endif // !AUTOPILOT_H
