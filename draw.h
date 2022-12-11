#ifndef DRAW_H
#define DRAW_H

#include "snake.h"
#include "structs.h"
#include "termbox.h"
// #include <curses.h>
// #include <locale.h>
#include <wchar.h>

#define TB_IMPL

enum styles { FANCY, FULL, ASCII, DOTS };

int init_scr(int selStyle, int *maxX, int *maxY, short arcadeMode,
             short dScore);
void draw_snake(Snake *snake);
void draw_food(Point food);
void draw_junk(List *junkList);
void draw_point(int x, int y, short color, int type);
void draw_score(Snake *snake);
void draw_walls();
#endif
