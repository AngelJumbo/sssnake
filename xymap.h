#ifndef XYMAP_H
#define XYMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum blocks { EMPTY, WALL, SBODY };
typedef struct xymap {

  short *arr;
  int maxX;
  int maxY;

} XYMap;

XYMap *xymap_create(int maxX, int maxY);
void xymap_free(XYMap *map);

void xymap_mark(XYMap *map, int x, int y, short type);
void xymap_unmark(XYMap *map, int x, int y);
int xymap_marked(XYMap *map, int x, int y);
void xymap_print(XYMap *map);

void xymap_print_log(XYMap *map, int headx, int heady, int tailx, int taily);
XYMap *xymap_copy(XYMap *map);
#endif
