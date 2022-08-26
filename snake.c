#include "snake.h"
#include "xymap.h"

SnakePart *snake_part_create(int x, int y, SnakePart *prev) {
  SnakePart *tmp = (SnakePart *)malloc(sizeof(SnakePart));
  tmp->prev = prev;
  tmp->next = NULL;
  tmp->x = x;
  tmp->y = y;
  return tmp;
}

void snake_part_free(SnakePart *snakePart) {
  if (snakePart != NULL) {
    free(snakePart);
  }
}

Snake *snake_create(int x, int y, int direction, short teleport) {
  Snake *tmp = (Snake *)malloc(sizeof(Snake));
  tmp->head = snake_part_create(x, y, NULL);
  tmp->tail = tmp->head;
  tmp->direction = direction;
  tmp->length = 1;
  tmp->grow = 1;
  tmp->collision = 0;
  tmp->teleport = teleport;
  return tmp;
}

void snake_free(Snake *snake) {
  if (snake != NULL) {
    SnakePart *part = snake->head;
    SnakePart *next;
    while (part != NULL) {
      next = part->next;
      snake_part_free(part);
      part = next;
    }
    free(snake);
  }
}

static void move_body(Snake *snake, XYMap *blocksTaken, Point *food, int prevx,
                      int prevy, int maxX, int maxY) {
  int prevx2, prevy2;
  if (snake->teleport) {
    if (snake->head->x >= maxX)
      snake->head->x = snake->head->x - maxX;
    if (snake->head->y >= maxY)
      snake->head->y = snake->head->y - maxY;
    if (snake->head->x < 0)
      snake->head->x = maxX + snake->head->x;
    if (snake->head->y < 0)
      snake->head->y = maxY + snake->head->y;
  }
  // xymap_unmark(blocksTaken, snake->tail->x, snake->tail->y);

  SnakePart *snakePart = snake->head->next;
  while (snakePart != NULL) {
    prevx2 = snakePart->x;
    prevy2 = snakePart->y;

    snakePart->x = prevx;
    snakePart->y = prevy;

    prevx = prevx2;
    prevy = prevy2;
    snakePart = snakePart->next;
  }
  if (food->x == snake->head->x && food->y == snake->head->y) {
    // rand_pos_food(food, blocksTaken, maxX, maxY);
    snake->grow++;
    snake->collision = 0;
  }

  if (snake->grow) {
    snake->grow--;
    snake->length++;
    snake->tail->next = snake_part_create(prevx, prevy, snake->tail);

    snake->tail = snake->tail->next;
    // xymap_mark(blocksTaken, snake->tail->x, snake->tail->y, SBODY);
    // rand_pos_food(food, blocksTaken, maxX, maxY);
  } else {
    // only unmark if the head haven't take the place of the tail
    xymap_unmark(blocksTaken, prevx, prevy);
  }
  xymap_mark(blocksTaken, snake->head->x, snake->head->y, SBODY);
}

void update_position(Snake *snake, XYMap *blocksTaken, Point *food, int dir,
                     int maxX, int maxY) {
  int prevx, prevy;
  int prevx2, prevy2;
  SnakePart *snakePart = snake->head;
  prevx = snakePart->x;
  prevy = snakePart->y;

  if (!((snake->direction == North && dir == South) ||
        (snake->direction == South && dir == North) ||
        (snake->direction == East && dir == West) ||
        (snake->direction == West && dir == East) || (dir == -1)))
    snake->direction = dir;

  switch (snake->direction) {
  case North:
    check_collision(snake, blocksTaken, snake->head->x, snake->head->y - 1,
                    maxX, maxY);
    snakePart->y--;
    break;
  case South:
    check_collision(snake, blocksTaken, snake->head->x, snake->head->y + 1,
                    maxX, maxY);
    snakePart->y++;
    break;
  case East:
    check_collision(snake, blocksTaken, snake->head->x + 1, snake->head->y,
                    maxX, maxY);
    snakePart->x++;
    break;
  case West:
    check_collision(snake, blocksTaken, snake->head->x - 1, snake->head->y,
                    maxX, maxY);
    snakePart->x--;
    break;
  }

  move_body(snake, blocksTaken, food, prevx, prevy, maxX, maxY);
}

void update_position_autopilot(Snake *snake, XYMap *blocksTaken, Point *food,
                               int x, int y, int maxX, int maxY) {
  int prevx, prevy;
  int prevx2, prevy2;
  SnakePart *snakePart = snake->head;
  prevx = snakePart->x;
  prevy = snakePart->y;
  check_collision(snake, blocksTaken, x, y, maxX, maxY);
  snake->head->x = x;
  snake->head->y = y;

  move_body(snake, blocksTaken, food, prevx, prevy, maxX, maxY);
}

void check_collision(Snake *sn, XYMap *blocksTaken, int x, int y, int maxX,
                     int maxY) {
  if (sn->teleport) {
    if (x >= maxX)
      x = x - maxX;
    if (y >= maxY)
      y = y - maxY;
    if (x < 0)
      x = maxX + x;
    if (y < 0)
      y = maxY + y;
  }
  if (x >= maxX || y >= maxY || x < 0 || y < 0)
    sn->collision = 3;
  else if (xymap_marked(blocksTaken, x, y)) {
    if (sn->tail->x == x && sn->tail->y == y)
      sn->collision = 0;
    else
      sn->collision = xymap_marked(blocksTaken, x, y);
  } else
    sn->collision = 0;
}

void rand_pos_food(Point *food, XYMap *blocksTaken, int maxX, int maxY) {
  int x, y;
  // xymap_unmark(blocksTaken, food->x, food->y);
  do {

    x = (int)(rand() % maxX);
    y = (int)(rand() % maxY);

  } while (xymap_marked(blocksTaken, x, y) || (food->x == x && food->y == y));

  // xymap_unmark(blocksTaken, food->x, food->y);
  // xymap_mark(blocksTaken, x, y);
  food->x = x;
  food->y = y;
}

Point *point_create(int x, int y) {
  Point *tmp = (Point *)malloc(sizeof(Point));
  tmp->x = x;
  tmp->y = y;
  return tmp;
}
