#include "xymap.h"
#include <stdio.h>

XYMap *xymap_create(int maxX, int maxY) {
  XYMap *tmp = (XYMap *)malloc(sizeof(XYMap));
  tmp->maxX = maxX;
  tmp->maxY = maxY;
  tmp->arr = (short *)malloc(sizeof(short) * maxX * maxY);

  for (int i = 0; i < maxX * maxY; i++)
    tmp->arr[i] = 0;
  return tmp;
}
void xymap_free(XYMap *map) {
  free(map->arr);
  free(map);
}
XYMap *xymap_copy(XYMap *map) {
  XYMap *tmp = xymap_create(map->maxX, map->maxY);
  for (int i = 0; i < map->maxX * map->maxY; i++) {
    tmp->arr[i] = map->arr[i];
  }
  return tmp;
}

void xymap_mark(XYMap *map, int x, int y, short type) {
  int pos = x + map->maxX * y;
  if (x >= 0 && y >= 0 && x < map->maxX && y < map->maxY)
    map->arr[pos] = type;
}
void xymap_unmark(XYMap *map, int x, int y) {

  int pos = x + map->maxX * y;
  if (x >= 0 && y >= 0 && x < map->maxX && y < map->maxY)
    map->arr[pos] = 0;
}
int xymap_marked(XYMap *map, int x, int y) {

  int pos = x + map->maxX * y;
  if (x >= 0 && y >= 0 && x < map->maxX && y < map->maxY)
    return map->arr[pos];
  return -1;
}

void xymap_print(XYMap *map) {

  for (int j = 0; j < map->maxY; j++) {
    for (int i = 0; i < map->maxX; i++)
      printf("%i ", map->arr[i + map->maxX * j]);

    printf("\n");
  }
}

void xymap_print_log(XYMap *map, int headx, int heady, int tailx, int taily) {
  FILE *fp;

  fp = fopen("log.txt", "a+");
  fprintf(fp, "   ");
  for (int i = 0; i < map->maxX; i++)
    fprintf(fp, "%i ", i);

  fprintf(fp, "\n");
  for (int i = 0; i < map->maxX; i++)
    fprintf(fp, "--");
  fprintf(fp, "\n");
  for (int j = 0; j < map->maxY; j++) {

    fprintf(fp, "%i| ", j);
    for (int i = 0; i < map->maxX; i++) {
      if (map->arr[i + map->maxX * j] != 0) {
        if (i == headx && j == heady) {
          fprintf(fp, "h ");
        } else if (i == tailx && j == taily) {
          fprintf(fp, "t ");
        } else {
          fprintf(fp, "O ");
        }
      } else {
        fprintf(fp, "  ");
      }
      // fprintf(fp, "%i ", map->arr[i + map->maxX * j]);
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");
  fclose(fp);
}
