#include "autopilot.h"
#include "snake.h"
#include "structs.h"
#include "xymap.h"

// static short selectedShortPathAlg = ASTAR;

static Stack *(*short_path)(XYMap *, Snake *, int, int, Point,
                            short) = a_star_search;

int (*calculate_heuristic)(int, int, int, int, Point, int, int,
                           short) = calculate_h_value;

void set_short_path_algorithm(short algorithm) {
  switch (algorithm) {
  case ASTAR:
    short_path = a_star_search;
    break;
  case BFS:
    short_path = breadth_first_search;
    break;
  case ASTARFIXED:
    short_path = a_star_search;
    calculate_heuristic = calculate_h_value_fixed;
    break;

  default:
    short_path = a_star_search;
    break;
  }
}

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

static int is_valid(int x, int y, int maxX, int maxY) {
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

  return !xymap_marked(xymap, x2, y2);

  // if (xymap_marked(xymap, x2, y2)) {
  //   return 0;
  // }
  // return 1;
}

static int is_destination(int x, int y, Point dest) {
  if (y == dest.y && x == dest.x)
    return 1;
  else
    return 0;
}

int calculate_h_value(int x, int y, int bx, int by, Point dest, int maxX,
                      int maxY, short teleport) {

  if (!teleport)
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

int calculate_h_value_fixed(int x, int y, int bx, int by, Point dest, int maxX,
                            int maxY, short teleport) {
  int diffy = abs(y - dest.y);
  int diffx = abs(x - dest.x);
  int h_value;
  if (diffx == 0 || diffy == 0)
    h_value = diffx + diffy;
  else if (x == bx || y == by)
    h_value = diffx + diffy;
  else
    return diffx + diffy + 1;

  if (teleport) {
    int min = h_value;
    int h2 = 0;
    int h3 = 0;
    if ((diffx == 0 || diffy == 0) || (x == bx || y == by)) {
      h2 = y > dest.y ? maxY - y + dest.y + abs(x - dest.x)
                      : y + maxY - dest.y + abs(x - dest.x);
      h3 = x > dest.x ? maxX - x + dest.x + abs(y - dest.y)
                      : x + maxX - dest.x + abs(y - dest.y);
    } else {

      h2 = y > dest.y ? maxY - y + dest.y + abs(x - dest.x) + 1
                      : y + maxY - dest.y + abs(x - dest.x) + 1;
      h3 = x > dest.x ? maxX - x + dest.x + abs(y - dest.y) + 1
                      : x + maxX - dest.x + abs(y - dest.y) + 1;
    }
    if (min > h2)
      min = h2;
    if (min > h3)
      min = h3;
    return min;
  }
  return h_value;
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
        hNew = calculate_heuristic(px, py, cellDetails[i + maxX * j]->parent_i,
                                   cellDetails[i + maxX * j]->parent_i, dest,
                                   maxX, maxY, snake->teleport);
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
        hNew = calculate_heuristic(px, py, cellDetails[i + maxX * j]->parent_i,
                                   cellDetails[i + maxX * j]->parent_i, dest,
                                   maxX, maxY, snake->teleport);
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
        hNew = calculate_heuristic(px, py, cellDetails[i + maxX * j]->parent_i,
                                   cellDetails[i + maxX * j]->parent_i, dest,
                                   maxX, maxY, snake->teleport);
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
        hNew = calculate_heuristic(px, py, cellDetails[i + maxX * j]->parent_i,
                                   cellDetails[i + maxX * j]->parent_i, dest,
                                   maxX, maxY, snake->teleport);
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

Stack *breadth_first_search(XYMap *m, Snake *snake, int maxX, int maxY,
                            Point dest, short checkBody) {
  // XYMap *m = xymap_copy(map);

  // xymap_unmark(m, dest.x, dest.y);
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
  i = snake->head->x, j = snake->head->y;
  pos = i + maxX * j;
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

  // xymap_unmark(m, i, j);
  Stack *path = NULL;

  Queue *q = queue_create();
  Point *p = NULL;

  queue_enqueue(q, point_create(snake->head->x, snake->head->y));
  closedList[i][j] = 1;
  while (q->count > 0) {
    p = (Point *)queue_dequeue(q);

    if (is_destination(p->x, p->y, dest)) {
      path = trace_path(cellDetails, dest, maxX);
      free(p);
      break;
    }

    for (i = -1; i < 2; i += 2) {
      for (j = 0; j < 2; j++) {
        int x = p->x + (j == 0 ? i : 0);
        int y = p->y + (j == 1 ? i : 0);
        if (snake->teleport) {
          if (x >= maxX)
            x = 0;
          if (y >= maxY)
            y = 0;
          if (x < 0)
            x = maxX - 1;
          if (y < 0)
            y = maxY - 1;
        }
        pos = x + maxX * y;
        if (is_valid(x, y, maxX, maxY)) {

          if (!closedList[x][y] && (is_unblocked(m, cellDetails, p->x, p->y, x,
                                                 y, maxX, checkBody) ||
                                    (dest.x == x && dest.y == y))) {
            // xymap_mark(m2, x, y, SBODY);
            closedList[x][y] = 1;
            cellDetails[pos]->parent_i = p->x;
            cellDetails[pos]->parent_j = p->y;
            Point *p2 = point_create(x, y);
            queue_enqueue(q, p2);
            if (checkBody)
              copy_and_move(cellDetails, p->x, p->y, x, y, maxX);
            // else
            // xymap_mark(m, x, y, SBODY);
          }
        }
      }
    }
    free(p);
    p = NULL;
    // if (p == NULL)
    // closedList[p->x][p->y] = 1;
  }

  if (path != NULL) {
    // path = stack_i /nvert(path);
    free(stack_pop(path));
  }
  queue_free(q);
  // xymap_free(m);
  // xymap_free(m2);

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

  free(cellDetails);
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

  int stepFound = 0;
  int maxX = xymap->maxX;
  int maxY = xymap->maxY;

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
    stepFound = 1;
  } else {

    stack_push(path, p2);
    stack_push(path, p1);
    xymap_mark(xymap, p1->x, p1->y, WALL);
    xymap_mark(xymap, p2->x, p2->y, WALL);
  }

  return stepFound;
}

static Stack *longer_path(XYMap *map, Stack *sPath, short teleport) {
  Stack *shortPath = copy_path(sPath);
  Stack *path = stack_create();
  Point *p1 = NULL;
  Point *p2 = NULL;
  int stepFound = 0;
  int count = 0;
  int maxExtend = 5;
  // Point *np1 = NULL;
  // Point *np2 = NULL;
  // Point head = *(Point *)sPath->last->data;
  XYMap *xymap = xymap_copy(map);
  // xymap_print_log(xymap, head.x, head.y, -1, -1);
  while (shortPath->last) {

    if (count < maxExtend)
      stepFound = long_step(xymap, shortPath, teleport);
    else
      break;

    if (!stepFound) {

      p1 = stack_pop(shortPath);
      p2 = stack_pop(shortPath);
      if (p1 != NULL) {
        stack_push(path, p1);
      }
      if (p2 != NULL) {
        stack_push(path, p2);
      }
    } else
      count++;
    stepFound = 0;
  }
  stack_free(shortPath);
  xymap_free(xymap);
  path = stack_invert(path);
  // xymap_print_log(xymap, head.x, head.y, -1, -1);
  return path;
}

Stack *try_hard(XYMap *xymap, Snake *snake, int maxX, int maxY, Point dest,
                short mode) {
  Snake *sn = snake_copy(snake);
  XYMap *map = xymap_copy(xymap);
  // Stack *path = a_star_search(map, sn, maxX, maxY, dest, 1);
  // Stack *path = breadth_first_search(map, sn, maxX, maxY, dest, 1);
  Stack *path = short_path(map, sn, maxX, maxY, dest, 1);
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
    // confirm = a_star_search(map, sn, maxX, maxY, tail, 0);
    // confirm = breadth_first_search(map, sn, maxX, maxY, tail, 0);
    confirm = short_path(map, sn, maxX, maxY, tail, 0);
    if (confirm != NULL) {
      safePathFound = 1;
      stack_free(confirm);
      // snake->onWayToFood = 1;
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
    // path2 = a_star_search(xymap, snake, maxX, maxY, tail, 0);
    path2 = short_path(xymap, snake, maxX, maxY, tail, 0);

    if (path2 != NULL) {
      Point *p = point_create(snake->head->x, snake->head->y);
      stack_push(path2, p);
      // map = xymap_copy(xymap);

      if (!mode)
        path = longer_path(xymap, path2, snake->teleport);
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

Stack *basic_path_search(XYMap *map, Snake *snake, int maxX, int maxY,
                         Point dest) {
  return short_path(map, snake, maxX, maxY, dest, 1);
}


/*
 def dfs_helper(x, y):
        if not is_valid(x, y):
            return False

        path.append((x, y))
        visited[x][y] = True

        if grid[x][y] == 'X':
            return False  # Hit an obstacle

        # Explore in all four directions
        for dx, dy in directions:
            new_x, new_y = x + dx, y + dy
            if dfs_helper(new_x, new_y):
                return True

        # If no path found, backtrack
        path.pop()
        return False

 */
/*
short dfs(XYMap *map,List *path, Point p, Point end_point, int path_len, int path_len_goal){

  if(path_len == path_len_goal+1 && p.x == end_point.x && p.y == end_point.y){
    list_append(path, point_create(p.x, p.y) );
    return 1;
  }
  if( !is_valid(p.x,p.y, map->maxX,map->maxY) )
    return 0;
  if ( xymap_marked(map,p.x,p.y) )
    return 0;
  list_append(path, point_create(p.x, p.y) );
  xymap_mark(map, p.x, p.y,SBODY);
  //xymap_print(map);
  xymap_print_log(map, p.x, p.y, end_point.x, end_point.y);


  Point new_points[]={
    {p.x-1, p.y},
    {p.x+1, p.y},
    {p.x, p.y-1},
    {p.x, p.y+1}
  };
  
  for(int i=0;i<4;i++){
    if( dfs(map,path,new_points[i], end_point, path_len+1, path_len_goal))
      return 1;

  }
  xymap_unmark(map, p.x, p.y);
  free(list_get_last(path)); 
  

  return 0;
}

List *hamiltonian_path(XYMap *map, Point initial_point){
  XYMap *map_copy = xymap_copy(map);
  xymap_unmark(map_copy, initial_point.x, initial_point.y);
  int path_len_goal = xymap_count_unmarked(map_copy);
  List *path = list_create();
  if(dfs(map_copy, path, initial_point, initial_point, 1, path_len_goal))
    return path;
  
  list_free(path);
  xymap_free(map_copy);
  return NULL;
   
}*/

int dfs(XYMap *map, List *path, Point p, Point end_point, int path_len_goal) {
    if (path_len_goal == 0 && p.x == end_point.x && p.y == end_point.y) {
        list_append(path, point_create(p.x, p.y));
        return 1;
    }

    if (!is_valid(p.x, p.y, map->maxX, map->maxY) || xymap_marked(map, p.x, p.y))
        return 0;

    list_append(path, point_create(p.x, p.y));
    xymap_mark(map, p.x, p.y, SBODY);

    int directions[][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

    for (int i = 0; i < 4; i++) {
        int new_x = p.x + directions[i][0];
        int new_y = p.y + directions[i][1];
        if (dfs(map, path, (Point){new_x, new_y}, end_point, path_len_goal - 1))
            return 1;
    }

    xymap_unmark(map, p.x, p.y);
    free(list_get_last(path)); 

    return 0;
}
/*
List *hamiltonian_path(XYMap *map, Point initial_point) {
    int path_len_goal = xymap_count_unmarked(map);
    List *path = list_create();

    if (dfs(map, path, initial_point, initial_point, path_len_goal))
        return path;

    list_free(path);
    return NULL;
}*/
List *hamiltonian_path(XYMap *map, Point initial_point){
  XYMap *map_copy = xymap_copy(map);
  xymap_unmark(map_copy, initial_point.x, initial_point.y);
  int path_len_goal = xymap_count_unmarked(map_copy);
  List *path = list_create();
  if(dfs(map_copy, path, initial_point, initial_point, path_len_goal))
    return path;
  
  list_free(path);
  xymap_free(map_copy);
  return NULL;
   
}


