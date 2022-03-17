#include "autopilot.h"
#include "snake.h"
#include "xymap.h"
// Most of this code is based on the code found in
// https://www.geeksforgeeks.org/a-search-algorithm/
// but written c instead of c++ with some extra stuff that consider the position
// of the snake body in each cell.
// I'll leave most of the geeksforgeeks comments, because I'm lazy
// (please don't sue me)

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

int is_unblocked(XYMap *xymap, Cell **cellDetails, int x, int y, int x2, int y2,
                 int maxX) {
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

int is_destination(int x, int y, Point dest) {
  if (y == dest.y && x == dest.x)
    return 1;
  else
    return 0;
}

int calculate_h_value(int x, int y, Point dest) {
  // not the distance formula because is dumb to use sqrt so much and we don't
  // need to be so precise
  return abs(y - dest.y) + abs(x - dest.x);
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

Stack *a_star_search(XYMap *xymap, Snake *snake, int maxX, int maxY, Point src,
                     Point dest) {
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

  cellDetails[pos]->head =
      snake_part_create(snake->head->x, snake->head->y, NULL);
  SnakePart *tmpPart = cellDetails[pos]->head;
  for (SnakePart *part = snake->head->next; part != NULL; part = part->next) {

    tmpPart->next = snake_part_create(part->x, part->y, tmpPart);
    tmpPart = tmpPart->next;
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

    //----------- 1st Successor (North) ------------

    // Only process this cell if this is a valid one
    if (is_valid(i - 1, j, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(i - 1, j, dest)) {
        // Set the Parent of the destination cell
        cellDetails[(i - 1) + maxX * j]->parent_i = i;
        cellDetails[(i - 1) + maxX * j]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }
      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[i - 1][j] &&
               is_unblocked(xymap, cellDetails, i, j, i - 1, j, maxX)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(i - 1, j, dest);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[(i - 1) + maxX * j]->f == FLT_MAX ||
            cellDetails[(i - 1) + maxX * j]->f > fNew) {
          minheap_add(openList, point_create(i - 1, j), fNew);

          // Update the details of this cell
          cellDetails[(i - 1) + maxX * j]->f = fNew;
          cellDetails[(i - 1) + maxX * j]->g = gNew;
          cellDetails[(i - 1) + maxX * j]->h = hNew;
          cellDetails[(i - 1) + maxX * j]->parent_i = i;
          cellDetails[(i - 1) + maxX * j]->parent_j = j;
          // cellDetails[(i - 1) + maxX * j]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i - 1, j);
          copy_and_move(cellDetails, i, j, i - 1, j, maxX);
        }
      }
    }
    //----------- 2nd Successor (South) ------------

    // Only process this cell if this is a valid one
    if (is_valid(i + 1, j, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(i + 1, j, dest)) {
        // Set the Parent of the destination cell
        cellDetails[(i + 1) + maxX * j]->parent_i = i;
        cellDetails[(i + 1) + maxX * j]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }
      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[i + 1][j] &&
               is_unblocked(xymap, cellDetails, i, j, i + 1, j, maxX)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(i + 1, j, dest);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[(i + 1) + maxX * j]->f == FLT_MAX ||
            cellDetails[(i + 1) + maxX * j]->f > fNew) {
          minheap_add(openList, point_create(i + 1, j), fNew);
          // openList.insert(make_pair(fNew, make_pair(i + 1, j)));
          // Update the details of this cell
          cellDetails[(i + 1) + maxX * j]->f = fNew;
          cellDetails[(i + 1) + maxX * j]->g = gNew;
          cellDetails[(i + 1) + maxX * j]->h = hNew;
          cellDetails[(i + 1) + maxX * j]->parent_i = i;
          cellDetails[(i + 1) + maxX * j]->parent_j = j;
          // cellDetails[(i + 1) + maxX * j]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i + 1, j);
          copy_and_move(cellDetails, i, j, i + 1, j, maxX);
        }
      }
    }

    //----------- 3rd Successor (East) ------------

    // Only process this cell if this is a valid one
    if (is_valid(i, j + 1, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(i, j + 1, dest)) {
        // Set the Parent of the destination cell
        cellDetails[i + maxX * (j + 1)]->parent_i = i;
        cellDetails[i + maxX * (j + 1)]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }

      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[i][j + 1] &&
               is_unblocked(xymap, cellDetails, i, j, i, j + 1, maxX)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(i, j + 1, dest);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[i + maxX * (j + 1)]->f == FLT_MAX ||
            cellDetails[i + maxX * (j + 1)]->f > fNew) {
          // openList.insert(make_pair(fNew, make_pair(i, j + 1)));
          minheap_add(openList, point_create(i, j + 1), fNew);

          // Update the details of this cell
          cellDetails[i + maxX * (j + 1)]->f = fNew;
          cellDetails[i + maxX * (j + 1)]->g = gNew;
          cellDetails[i + maxX * (j + 1)]->h = hNew;
          cellDetails[i + maxX * (j + 1)]->parent_i = i;
          cellDetails[i + maxX * (j + 1)]->parent_j = j;
          // cellDetails[i + maxX * (j + 1)]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i, j + 1);
          copy_and_move(cellDetails, i, j, i, j + 1, maxX);
        }
      }
    }

    //----------- 4th Successor (West) ------------

    // Only process this cell if this is a valid one
    if (is_valid(i, j - 1, maxX, maxY)) {
      // If the destination cell is the same as the
      // current successor
      if (is_destination(i, j - 1, dest)) {
        // Set the Parent of the destination cell
        cellDetails[i + maxX * (j - 1)]->parent_i = i;
        cellDetails[i + maxX * (j - 1)]->parent_j = j;
        // printf("The destination cell is found\n");
        path = trace_path(cellDetails, dest, maxX);
        break;
      }

      // If the successor is already on the closed
      // list or if it is blocked, then ignore it.
      // Else do the following
      else if (!closedList[i][j - 1] &&
               is_unblocked(xymap, cellDetails, i, j, i, j - 1, maxX)) {
        gNew = cellDetails[i + maxX * j]->g + 1.0;
        hNew = calculate_h_value(i, j - 1, dest);
        fNew = gNew + hNew;

        // If it isn’t on the open list, add it to
        // the open list. Make the current square
        // the parent of this square. Record the
        // f, g, and h costs of the square cell
        //			 OR
        // If it is on the open list already, check
        // to see if this path to that square is
        // better, using 'f' cost as the measure.
        if (cellDetails[i + maxX * (j - 1)]->f == FLT_MAX ||
            cellDetails[i + maxX * (j - 1)]->f > fNew) {
          // openList.insert(make_pair(fNew, make_pair(i, j - 1)));
          minheap_add(openList, point_create(i, j - 1), fNew);

          // Update the details of this cell
          cellDetails[i + maxX * (j - 1)]->f = fNew;
          cellDetails[i + maxX * (j - 1)]->g = gNew;
          cellDetails[i + maxX * (j - 1)]->h = hNew;
          cellDetails[i + maxX * (j - 1)]->parent_i = i;
          cellDetails[i + maxX * (j - 1)]->parent_j = j;
          // cellDetails[i + maxX * (j - 1)]->head =
          // copy_and_move(cellDetails[i + maxX * j]->head, i, j - 1);
          copy_and_move(cellDetails, i, j, i, j - 1, maxX);
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

  return path;
}
