#include "autopilot.h"
#include "essentials.h"
#include "snake.h"
#include "xymap.h"
#include <stdio.h>

Cell *cell_create(int parent_i, int parent_j, double f, double g, double h) {
  Cell *tmp = (Cell *)malloc(sizeof(Cell));
  tmp->parent_i = parent_i;
  tmp->parent_j = parent_j;
  tmp->f = f;
  tmp->g = g;
  tmp->h = h;
  tmp->head = NULL;
  return tmp;
}

static SnakePart *copy_snake_body(SnakePart *head, int x, int y) {

  SnakePart *tmp = snake_part_create(x, y, NULL);
  SnakePart *tmp2 = tmp;
  for (SnakePart *part = head; part->next != NULL; part = part->next) {
    tmp2->next = snake_part_create(part->x, part->y, tmp2);
    tmp2 = tmp2->next;
  }
  return tmp;
}

static void copy_and_move(Cell **cellDetails, int x, int y, int x2, int y2,
                          int maxX) {
  int pos1 = x + maxX * y;
  int pos2 = x2 + maxX * y2;
  if (cellDetails[pos2]->head != NULL) {
    SnakePart *part = cellDetails[pos2]->head;
    SnakePart *part2 = NULL;
    while (part != NULL) {
      part2 = part->next;
      free(part);
      part = part2;
    }
  }
  cellDetails[pos2]->head = copy_snake_body(cellDetails[pos1]->head, x2, y2);
}

int is_valid(int x, int y, int maxX, int maxY) {
  return (y >= 0) && (y < maxY) && (x >= 0) && (x < maxX);
}

static int is_unblocked(XYMap *xymap, Cell **cellDetails, int x, int y, int x2,
                        int y2, int maxX, int checkBody) {
  if (checkBody) {
    if (xymap_marked(xymap, x2, y2) == WALL) {
      return 0;
    }
    SnakePart *head = cellDetails[x + maxX * y]->head;

    int unblocked = 1;

    for (SnakePart *part = head; part->next != NULL; part = part->next) {
      if (part->x == x2 && part->y == y2) {
        unblocked = 0;
        break;
      }
    }

    return unblocked;
  }
  if (xymap_marked(xymap, x2, y2)) {
    return 0;
  }
  return 1;
}

int is_destination(int x, int y, Point dest) {
  if (y == dest.y && x == dest.x)
    return 1;
  else
    return 0;
}

int calculate_h_value(int x, int y, Point dest, int maxX, int maxY,
                      short teleport) {
  if (!teleport)
    // not the distance formula because is dumb to use sqrt so much and we don't
    // need to be so precise
    return abs(y - dest.y) + abs(x - dest.x);
  else {
    int min = abs(y - dest.y) + abs(x - dest.x);
    int h2 = y > dest.y ? maxY - y + dest.y + abs(x - dest.x)
                        : y + maxY - dest.y + abs(x - dest.x);
    int h3 = x > dest.x ? maxX - x + dest.x + abs(y - dest.y)
                        : x + maxX - dest.x + abs(y - dest.y);
    if (min > h2)
      min = h2;
    if (min > h3)
      min = h3;
    return min;
  }
}

Stack *trace_path(Cell **cellDetails, Point dest, int maxX) {
  int y = dest.y;
  int x = dest.x;
  int pos = x + maxX * y;
  // stack<Pair> Path;
  Stack *Path = stack_create();

  while (
      !(cellDetails[pos]->parent_i == x && cellDetails[pos]->parent_j == y)) {
    stack_push(Path, point_create(x, y));
    int temp_x = cellDetails[pos]->parent_i;
    int temp_y = cellDetails[pos]->parent_j;
    y = temp_y;
    x = temp_x;
    pos = x + maxX * y;
  }

  stack_push(Path, point_create(x, y));

  return Path;
}

Stack *a_star_search(XYMap *xymap, Snake *snake, int maxX, int maxY, Point dest,
                     short checkBody) {
  Point src = {snake->head->x, snake->head->y};
  // If the source is out of range
  // if (!is_valid(src.x, src.y, maxX, maxY)) {
  // printf("Source is invalid\n");
  // return NULL;
  // }

  // If the destination is out of range
  // if (!is_valid(dest.x, dest.y, maxX, maxY)) {
  // printf("Destination is invalid\n");
  // return NULL;
  // }

  // Either the source or the destination is blocked
  // if (!(is_unblocked(xymap, src.x, src.y) &&
  // is_unblocked(xymap, dest.x, dest.y))) {
  // printf("Source or the destination is blocked\n");
  // return NULL;
  // }

  // If the destination cell is the same as source cell
  // if (is_destination(src.x, src.y, dest)) {
  // printf("We are already at the destination\n");
  // return NULL;
  // }

  // Create a closed list and initialise it to false which
  // means that no cell has been included yet This closed
  // list is implemented as a boolean 2D array
  int closedList[maxX][maxY];
  memset(closedList, 0, sizeof(closedList));

  // Declare a 2D array of structure to hold the details
  // of that cell
  Cell **cellDetails = (Cell **)malloc(sizeof(void *) * maxX * maxY);

  int i, j;
  int pos;
  for (i = 0; i < maxX; i++) {
    for (j = 0; j < maxY; j++) {
      pos = i + maxX * j;
      cellDetails[pos] = cell_create(-1, -1, FLT_MAX, FLT_MAX, FLT_MAX);
    }
  }

  // Initialising the parameters of the starting node
  i = src.x, j = src.y;
  pos = i + maxX * j;
  cellDetails[pos]->f = 0.0;
  cellDetails[pos]->g = 0.0;
  cellDetails[pos]->h = 0.0;
  cellDetails[pos]->parent_i = i;
  cellDetails[pos]->parent_j = j;

  if (checkBody) {
    cellDetails[pos]->head =
        snake_part_create(snake->head->x, snake->head->y, NULL);
    SnakePart *tmpPart = cellDetails[pos]->head;
    for (SnakePart *part = snake->head->next; part != NULL; part = part->next) {

      tmpPart->next = snake_part_create(part->x, part->y, tmpPart);
      tmpPart = tmpPart->next;
    }
  }
  /*
  Create an open list having information as-
  <f, <i, j>>
  where f = g + h,
  and i, j are the y and xumn index of that cell
  Note that 0 <= i <= MAXY-1 & 0 <= j <= MAXX-1
  This open list is implemented as a set of pair of
  pair.*/
  MinHeap *openList = minheap_create();

  // Put the starting cell on the open list and set its
  // 'f' as 0
  minheap_add(openList, point_create(i, j), 0);

  // We set this boolean value as false as initially
  // the destination is not reached.
  int foundDest = 0;
  Stack *path = NULL;

  while (openList->count > 0) {
    Point *p = (Point *)minheap_remove_min(openList);

    // Remove this vertex from the open list
    // openList.erase(openList.begin());

    // Add this vertex to the closed list
    i = p->x;
    j = p->y;
    free(p);
    closedList[i][j] = 1;

    /*
    Generating all the 4 successor of this cell

                      N
                      |
                 W--Cell--E
                      |
                      S

    Cell-->Popped Cell (i, j)
    N --> North	 (i-1, j)
    S --> South	 (i+1, j)
    E --> East	 (i, j+1)
    W --> West		 (i, j-1)
    */
    // To store the 'g', 'h' and 'f' of the 4 successors
    double gNew, hNew, fNew;
    int px, py;
    //----------- 1st Successor (North) ------------

    // Only process this cell if this is a valid one
    px = i - 1;
    py = j;
    if (snake->teleport && px < 0)
      px = maxX - 1;

    if (is_valid(px, py, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(px, py, dest)) {
        // Set the Parent of the destination cell
        cellDetails[px + maxX * py]->parent_i = i;
        cellDetails[px + maxX * py]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }
      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[px][py] && is_unblocked(xymap, cellDetails, i, j, px,
                                                   py, maxX, checkBody)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(px, py, dest, maxX, maxY, snake->teleport);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[px + maxX * py]->f == FLT_MAX ||
            cellDetails[px + maxX * py]->f > fNew) {
          minheap_add(openList, point_create(px, py), fNew);

          // Update the details of this cell
          cellDetails[px + maxX * py]->f = fNew;
          cellDetails[px + maxX * py]->g = gNew;
          cellDetails[px + maxX * py]->h = hNew;
          cellDetails[px + maxX * py]->parent_i = i;
          cellDetails[px + maxX * py]->parent_j = j;
          // cellDetails[(i - 1) + maxX * j]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i - 1, j);
          if (checkBody)
            copy_and_move(cellDetails, i, j, px, py, maxX);
        }
      }
    }
    //----------- 2nd Successor (South) ------------

    // Only process this cell if this is a valid one
    px = i + 1;
    py = j;
    if (snake->teleport && px == maxX)
      px = 0;
    if (is_valid(px, py, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(px, py, dest)) {
        // Set the Parent of the destination cell
        cellDetails[px + maxX * py]->parent_i = i;
        cellDetails[px + maxX * py]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }
      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[px][py] && is_unblocked(xymap, cellDetails, i, j, px,
                                                   py, maxX, checkBody)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(px, py, dest, maxX, maxY, snake->teleport);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[px + maxX * py]->f == FLT_MAX ||
            cellDetails[px + maxX * py]->f > fNew) {
          minheap_add(openList, point_create(px, py), fNew);
          // openList.insert(make_pair(fNew, make_pair(i + 1, j)));
          // Update the details of this cell
          cellDetails[px + maxX * py]->f = fNew;
          cellDetails[px + maxX * py]->g = gNew;
          cellDetails[px + maxX * py]->h = hNew;
          cellDetails[px + maxX * py]->parent_i = i;
          cellDetails[px + maxX * py]->parent_j = j;
          // cellDetails[(i + 1) + maxX * j]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i + 1, j);
          if (checkBody)
            copy_and_move(cellDetails, i, j, px, py, maxX);
        }
      }
    }

    //----------- 3rd Successor (East) ------------

    // Only process this cell if this is a valid one
    px = i;
    py = j + 1;
    if (snake->teleport && py == maxY)
      py = 0;
    if (is_valid(px, py, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(px, py, dest)) {
        // Set the Parent of the destination cell
        cellDetails[px + maxX * py]->parent_i = i;
        cellDetails[px + maxX * py]->parent_j = j;
        path = trace_path(cellDetails, dest, maxX);
        // printf("The destination cell is found\n");
        break;
      }

      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[px][py] && is_unblocked(xymap, cellDetails, i, j, px,
                                                   py, maxX, checkBody)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(px, py, dest, maxX, maxY, snake->teleport);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[px + maxX * py]->f == FLT_MAX ||
            cellDetails[px + maxX * py]->f > fNew) {
          // openList.insert(make_pair(fNew, make_pair(i, j + 1)));
          minheap_add(openList, point_create(px, py), fNew);

          // Update the details of this cell
          cellDetails[px + maxX * py]->f = fNew;
          cellDetails[px + maxX * py]->g = gNew;
          cellDetails[px + maxX * py]->h = hNew;
          cellDetails[px + maxX * py]->parent_i = i;
          cellDetails[px + maxX * py]->parent_j = j;
          // cellDetails[i + maxX * (j + 1)]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i, j + 1);
          if (checkBody)
            copy_and_move(cellDetails, i, j, px, py, maxX);
        }
      }
    }

    //----------- 4th Successor (West) ------------

    // Only process this cell if this is a valid one
    px = i;
    py = j - 1;

    if (snake->teleport && py < 0)
      py = maxY - 1;

    if (is_valid(px, py, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(px, py, dest)) {
        // Set the Parent of the destination cell
        cellDetails[px + maxX * py]->parent_i = i;
        cellDetails[px + maxX * py]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }

      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[px][py] && is_unblocked(xymap, cellDetails, i, j, px,
                                                   py, maxX, checkBody)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(px, py, dest, maxX, maxY, snake->teleport);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[px + maxX * py]->f == FLT_MAX ||
            cellDetails[px + maxX * py]->f > fNew) {
          // openList.insert(make_pair(fNew, make_pair(i, j - 1)));
          minheap_add(openList, point_create(px, py), fNew);

          // Update the details of this cell
          cellDetails[px + maxX * py]->f = fNew;
          cellDetails[px + maxX * py]->g = gNew;
          cellDetails[px + maxX * py]->h = hNew;
          cellDetails[px + maxX * py]->parent_i = i;
          cellDetails[px + maxX * py]->parent_j = j;
          // cellDetails[i + maxX * (j - 1)]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i, j - 1);
          if (checkBody)
            copy_and_move(cellDetails, i, j, px, py, maxX);
        }
      }
    }
  }
  for (i = 0; i < maxX; i++) {
    for (j = 0; j < maxY; j++) {
      pos = i + maxX * j;

      if (cellDetails[pos]->head != NULL) {
        SnakePart *part = cellDetails[pos]->head;
        SnakePart *partNext = NULL;
        while (part != NULL) {
          partNext = part->next;
          free(part);
          part = partNext;
        }
      }
      free(cellDetails[pos]);
    }
  }
  minheap_free(openList);
  free(cellDetails);
  // When the destination cell is not found and the open
  // list is empty, then we conclude that we failed to
  // reach the destination cell. This may happen when the
  // there is no way to destination cell (due to
  // blockages)
  if (path != NULL)
    free(stack_pop(path));
  return path;
}
static Stack *copy_path(Stack *stack) {
  Node *node = stack->last;
  Point *ptmp = NULL;
  if (node != NULL) {
    ptmp = (Point *)node->data;
    Stack *tmp = stack_create();
    tmp->last = node_create(point_create(ptmp->x, ptmp->y));
    node = node->next;
    Node *node2 = tmp->last;
    while (node != NULL) {

      ptmp = (Point *)node->data;
      node2->next = node_create(point_create(ptmp->x, ptmp->y));
      node = node->next;
      node2 = node2->next;
    }
    tmp->count = stack->count;
    return tmp;
  }
  return NULL;
}

int long_step(XYMap *xymap, Stack *path, short teleport) {

  Point *p1 = stack_pop(path);

  if (p1 == NULL)
    return 0;
  Point *p2 = stack_pop(path);
  if (p2 == NULL) {
    stack_push(path, p1);
    return 0;
  }
  Point *p3 = stack_pop(path);
  if (p3 != NULL) {

    stack_push(path, p3);
    xymap_mark(xymap, p3->x, p3->y, WALL);
  }
  Point *np1 = NULL;
  Point *np2 = NULL;

  // XYMap *xymap = xymap_copy(map);
  int stepFound = 0;
  // Stack *path = stack_create();
  int maxX = xymap->maxX;
  int maxY = xymap->maxY;
  // Point *p1 = (Point *)stack_pop(shortPath);
  // Point *p2 = (Point *)stack_pop(shortPath);

  int x1, x2, y1, y2;

  if (p1->x == p2->x) {
    x1 = x2 = p2->x + 1;
    if (teleport && x1 >= maxX)
      x1 = x2 = 0;

    y1 = p1->y;
    y2 = p2->y;
    if (is_valid(x1, y1, maxX, maxY) && !xymap_marked(xymap, x1, y1) &&
        !xymap_marked(xymap, x2, y2)) {
      np1 = point_create(x1, y1);
      np2 = point_create(x2, y2);

      // stack_push(path, p1);
    } else {
      x1 = x2 = p2->x - 1;
      if (teleport && x1 < 0)
        x1 = x2 = maxX - 1;
      y1 = p1->y;
      y2 = p2->y;
      if (is_valid(x1, y1, maxX, maxY) && !xymap_marked(xymap, x1, y1) &&
          !xymap_marked(xymap, x2, y2)) {
        np1 = point_create(x1, y1);
        np2 = point_create(x2, y2);
        // stack_push(path, point_create(x1, y1));
      }
    }
  }
  if (p1->y == p2->y) {
    x1 = p1->x;
    x2 = p2->x;

    y1 = y2 = p1->y + 1;
    if (teleport && y1 >= maxY)
      y1 = y2 = 0;

    if (is_valid(x1, y1, maxX, maxY) && !xymap_marked(xymap, x1, y1) &&
        !xymap_marked(xymap, x2, y2)) {
      np1 = point_create(x1, y1);
      np2 = point_create(x2, y2);
    } else {
      x1 = p1->x;
      x2 = p2->x;

      y1 = y2 = p1->y - 1;
      if (teleport && y1 < 0)
        y1 = y2 = maxY - 1;

      if (is_valid(x1, y1, maxX, maxY) && !xymap_marked(xymap, x1, y1) &&
          !xymap_marked(xymap, x2, y2)) {
        np1 = point_create(x1, y1);
        np2 = point_create(x2, y2);
      }
    }
  }
  if (np1 != NULL && np2 != NULL) {

    stack_push(path, p2);
    stack_push(path, np2);
    stack_push(path, np1);
    stack_push(path, p1);
    xymap_mark(xymap, p2->x, p2->y, 3);
    xymap_mark(xymap, np2->x, np2->y, 4);
    xymap_mark(xymap, np1->x, np1->y, 5);
    xymap_mark(xymap, p1->x, p1->y, 6);
    // xymap_print_log(xymap, -1, -1, -1, -1);
    stepFound = 1;
  } else {

    stack_push(path, p2);
    stack_push(path, p1);
    xymap_mark(xymap, p1->x, p1->y, WALL);
    xymap_mark(xymap, p2->x, p2->y, WALL);
    // xymap_print_log(xymap, -1, -1, -1, -1);
  }

  return stepFound;
  // xymap_free(xymap);
  // stack_push(shortPath, p2);
  // stack_push(shortPath, p1);
}
static Stack *stack_invert2(Stack *stack, XYMap *map) {
  Stack *tmp = stack_create();
  tmp->count = stack->count;
  Point *p = NULL;
  int count = 3;
  while (stack->last != NULL) {
    p = (Point *)stack_pop(stack);
    xymap_mark(map, p->x, p->y, count);
    stack_push(tmp, p);
    count++;
  }
  stack_free(stack);
  return tmp;
}
static Stack *longest_path(XYMap *map, Stack *sPath, short teleport) {
  Stack *shortPath = copy_path(sPath);
  Stack *path = stack_create();
  Point *p1 = NULL;
  Point *p2 = NULL;
  int stepFound = 0;
  // Point *np1 = NULL;
  // Point *np2 = NULL;
  Point head = *(Point *)sPath->last->data;
  XYMap *xymap = xymap_copy(map);
  // xymap_print_log(xymap, head.x, head.y, -1, -1);
  while (shortPath->last) {
    // p1 = stack_pop(shortPath);
    // p2 = stack_pop(shortPath);

    // if (p1 != NULL && p2 != NULL) {
    // xymap_mark(xymap, p2->x, p2->y, WALL);
    // xymap_mark(xymap, p1->x, p1->y, WALL);
    stepFound = long_step(xymap, shortPath, teleport);
    // xymap_mark(xymap, p1->x, p1->y, WALL);
    // xymap_mark(xymap, p2->x, p2->y, WALL);
    // xymap_unmark(xymap, p2->x, p2->y);
    // xymap_print_log(xymap, head.x, head.y, -1, -1);
    // }
    /*
        if (np1 != NULL) {
          stack_push(shortPath, p2);
          stack_push(shortPath, np2);
          stack_push(shortPath, np1);
          stack_push(shortPath, p1);
          xymap_mark(xymap, p2->x, p2->y, WALL);
          xymap_mark(xymap, p1->x, p1->y, WALL);
          xymap_mark(xymap, np2->x, np2->y, WALL);
          xymap_mark(xymap, np1->x, np1->y, WALL);
          // xymap_mark(xymap, p2->x, p2->y, SBODY);
          np1 = NULL;
          np2 = NULL;
          // xymap_unmark(xymap, p1->x, p1->y);
          // xymap_unmark(xymap, p2->x, p2->y);
          // xymap_mark(xymap, p2->x, p2->y, WALL);
          xymap_print_log(xymap, head.x, head.y, -1, -1);
        } else
      */
    if (!stepFound) {

      p1 = stack_pop(shortPath);
      p2 = stack_pop(shortPath);
      if (p1 != NULL) {
        stack_push(path, p1);
      }
      if (p2 != NULL) {
        stack_push(path, p2);
      }
    }
  }
  path = stack_invert2(path, xymap);
  // xymap_print_log(xymap, head.x, head.y, -1, -1);
  return path;
}

Stack *try_hard(XYMap *xymap, Snake *snake, int maxX, int maxY, Point dest,
                short mode) {
  Snake *sn = snake_copy(snake);
  XYMap *map = xymap_copy(xymap);
  Stack *path = a_star_search(map, sn, maxX, maxY, dest, 1);
  Stack *confirm = NULL;
  Stack *path2 = NULL;
  int safePathFound = 0;
  if (path != NULL) {
    path2 = copy_path(path);
    while (path2->last != NULL) {

      Point *p = (Point *)stack_pop(path2);
      update_position_autopilot(sn, map, &dest, p->x, p->y, maxX, maxY);
      free(p);
    }
    Point tail = {sn->tail->x, sn->tail->y};
    confirm = a_star_search(map, sn, maxX, maxY, tail, 0);
    if (confirm != NULL) {
      safePathFound = 1;
      stack_free(confirm);
      snake->onWayToFood = 1;
    }
    stack_free(path2);
  }
  xymap_free(map);
  snake_free(sn);
  if (safePathFound == 0) {
    if (path != NULL)
      stack_free(path);
    path = NULL;
    Point tail = {snake->tail->x, snake->tail->y};
    path2 = a_star_search(xymap, snake, maxX, maxY, tail, 0);
    // if (path2 != NULL) {
    //   path = stack_create();
    //   stack_push(path, stack_pop(path2));
    //   stack_free(path2);
    // }
    if (path2 != NULL) {
      Point *p = point_create(snake->head->x, snake->head->y);
      stack_push(path2, p);
      // map = xymap_copy(xymap);

      if (!mode)
        path = longest_path(xymap, path2, snake->teleport);
      else {

        map = xymap_copy(xymap);
        long_step(map, path2, snake->teleport);
        xymap_free(map);
      }

      // xymap_free(map);
      if (path != NULL) {
        free(stack_pop(path));
        stack_free(path2);

      } else {

        free(stack_pop(path2));
        path = stack_create();
        stack_push(path, stack_pop(path2));
        stack_free(path2);
      }

      // snake_free(sn);
      // free(stack_pop(path));

      // path = path2;
    }
  }
  return path;
}
