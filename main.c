#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "autopilot.h"
#include "draw.h"
#include "snake.h"
#include "structs.h"
#include "xymap.h"

enum modes { NORMAL, ARCADE, AUTOPILOT, SCREENSAVER };

static int direction = East;
static int maxX = 0;
static int maxY = 0;
static int c = East;
static clock_t initTime;

static int selectedStyle = FANCY;
static int selectedMode = NORMAL;
static int selectedAlgorithm = BASIC;
static int speed = 200000;
static int junk = 0;
static short score = 0;
static short teleport = 0;

void init_options(int argc, char *argv[]);
int get_direction(int c);
int check_junk_pos(XYMap *blocksTaken, int x, int y);
void print_help();
int check_path(Stack *path);

int main(int argc, char *argv[]) {

  XYMap *blocksTaken = NULL;
  Snake *snake = NULL;
  Stack *path = NULL;
  List *junkList = NULL;
  Point food = {-1, -1};

  // Control variables for the screensaver mode

  Point controlLastPoint = {-1, -1};
  int controlSnakeSize = 0;
  int loops = 0;

  init_options(argc, argv);

  srand(time(NULL));

  score = score || selectedMode == ARCADE;

  if (!init_scr(selectedStyle, &maxX, &maxY, selectedMode == ARCADE, score))
    return 0;

  int maxBlocks = maxX * maxY;
  int junkCount = 0;
  do { // This loop goes forever if the option "screensaver" is given.

    draw_walls();
    //  Used to know where the body of the snake and the junk are,
    //  blocksTaken is just a matrix of 2x2 but It can change its dimesions
    //  dinamically if I ever decide to change  them by detecting changes in the
    //  dimensions of the terminal.
    blocksTaken = xymap_create(maxX, maxY);
    snake = snake_create(maxX / 2, maxY / 2, direction, teleport);
    xymap_mark(blocksTaken, snake->head->x, snake->head->y, SBODY);
    // A snake with a size of just one cell may break somethings
    // so just grow it by one at the start.
    snake->grow = 1;

    // Self explainatory block (I hope)
    if (junk) {
      junkList = list_create();
      int maxJunk = (maxX * maxY) * junk / 100;
      for (int i = 0; i < maxJunk; i++) {
        int jx, jy;
        do {

          jx = (rand() % (maxX - 2)) + 1;
          jy = (rand() % (maxY - 2)) + 1;
        } while (
            !check_junk_pos(blocksTaken, jx, jy)); // avoid snake spawn blocks
        xymap_mark(blocksTaken, jx, jy, WALL);
        list_append(junkList, point_create(jx, jy));
      }

      draw_junk(junkList);
      junkCount = junkList->count;
    }

    rand_pos_food(&food, blocksTaken, maxX, maxY);

    // The game loop
    while (1) {
      // Get a new path to follow if the autopilot or the screensaver are active
      if ((selectedMode == AUTOPILOT || selectedMode == SCREENSAVER) &&
          !check_path(path)) {

        switch (selectedAlgorithm) {
        case BASIC:

          path = a_star_search(blocksTaken, snake, maxX, maxY, food, 1);
          // path = breadth_first_search(blocksTaken, snake, maxX, maxY, food,
          // 1);
          break;
        case GREEDY1:
          path = try_hard(blocksTaken, snake, maxX, maxY, food, 0);
          break;
        case GREEDY2:
          path = try_hard(blocksTaken, snake, maxX, maxY, food, 1);
          break;
        }

        if (path == NULL && snake->length > 1) {
          // if there is no path found then mark the direction to which the
          // snake was headed and can follow it to die
          if ((snake->head->x - (snake->head->next->x + 1)) == 0)
            direction = East;
          if ((snake->head->x - (snake->head->next->x - 1)) == 0)
            direction = West;
          if ((snake->head->y - (snake->head->next->y + 1)) == 0)
            direction = South;
          if ((snake->head->y - (snake->head->next->y - 1)) == 0)
            direction = North;
        }
      }

      timeout(0);
      c = getch();

      // Pause game
      if (c == ' ') {
        do {
          nodelay(stdscr, false);
          c = getch();
        } while (c != ' ');

        nodelay(stdscr, true);
        c = getch();
      }

      if (path != NULL) {

        Point *point = stack_pop(path);
        update_position_autopilot(snake, blocksTaken, &food, point->x, point->y,
                                  maxX, maxY);
        free(point);
      } else {
        // If the autopilot is on and there is no path to follow then let the
        // snake die.
        if (!(selectedMode == AUTOPILOT || selectedMode == SCREENSAVER))
          direction = get_direction(c);
        update_position(snake, blocksTaken, &food, direction, maxX, maxY);
      }

      // For the screensaver mode we need to break the loop
      if (selectedMode == SCREENSAVER) {
        if (controlLastPoint.x == snake->head->x &&
            controlLastPoint.y == snake->head->y &&
            snake->length == controlSnakeSize) {
          loops++;
          if (loops > 1) {
            stack_free(path);
            path = a_star_search(blocksTaken, snake, maxX, maxY, food, 1);
          }
        }
        if (snake->length == maxBlocks)
          break;
      }
      if (snake->head->x == food.x && snake->head->y == food.y) {
        stack_free(path);
        path = NULL;
        if (junkCount + snake->length < maxBlocks) {
          controlLastPoint = food;
          controlSnakeSize = snake->length;
          loops = 0;
          rand_pos_food(&food, blocksTaken, maxX, maxY);

        } else {
          food.x = -1;
          food.y = -1;
        }

        if (selectedMode == ARCADE)
          speed = speed - 2000;
      }
      if (snake->collision || c == 'q') {
        break;
      }
      if (selectedMode == SCREENSAVER && c != ERR) {
        c = 'q';
        break;
      }
      draw_snake(snake);
      draw_food(food);
      if (score)
        draw_score(snake);
      c = 0;
      refresh();
      usleep(speed);
    }

    clear();
    refresh();
    if (junk)
      list_free(junkList);
    snake_free(snake);
    xymap_free(blocksTaken);
    if (path != NULL)
      stack_free(path);
    if (c == 'q')
      break;
  } while (selectedMode == SCREENSAVER);
  endwin();
}

int check_path(Stack *path) {

  if (path != NULL) {
    if (path->last == NULL) {
      stack_free(path);
      path = NULL;
      return 0;
    } else
      return 1;
  }
  return 0;
}

int check_junk_pos(XYMap *blocksTaken, int x, int y) {

  if (xymap_marked(blocksTaken, x, y) ||
      (y == maxY / 2 && (x > maxX / 4 && y < maxX * 3 / 4)))
    return 0;
  int count = 0;
  // avoid this shape:
  //
  //    ▀   ▀
  //      ▀

  for (int i = -1; i < 2; i += 2) {

    for (int j = -1; j < 2; j += 2) {
      if (xymap_marked(blocksTaken, x + i, y + j)) {

        count++;

        if (xymap_marked(blocksTaken, x + i * 2, y)) {
          count++;
        }
        if (xymap_marked(blocksTaken, x, y + j * 2)) {
          count++;
        }
      }
    }
  }
  if (count >= 2)
    return 0;
  return 1;
}
int get_direction(int c) {
  switch (c) {
  case KEY_UP:
  case 'k':
  case 'K':
  case 'w':
  case 'W':
    return North;
    break;
  case KEY_DOWN:
  case 'j':
  case 'J':
  case 's':
  case 'S':
    return South;
    break;
  case KEY_LEFT:
  case 'a':
  case 'A':
  case 'h':
  case 'H':
    return West;
    break;
  case KEY_RIGHT:
  case 'd':
  case 'D':
  case 'l':
  case 'L':
    return East;
    break;
  default:
    return -1;
    break;
  }
  return -1;
}

void init_options(int argc, char *argv[]) {
  int op;
  int speedMult;

  const struct option long_options[] = {{"help", 0, NULL, 'h'},
                                        {"speed", 1, NULL, 's'},
                                        {"screensaver", 0, NULL, 'S'},
                                        {"fancy", 0, NULL, 'f'},
                                        {"junk", 1, NULL, 'j'},
                                        {"autopilot", 0, NULL, 'a'},
                                        {"ascii", 0, NULL, 'A'},
                                        {"score", 0, NULL, 'z'},
                                        {"look", 1, NULL, 'l'},
                                        {"mode", 1, NULL, 'm'},
                                        {"teleport", 0, NULL, 't'},
                                        {"maxX", 1, NULL, 'x'},
                                        {"maxY", 1, NULL, 'y'},
                                        {"try-hard", 1, NULL, 1},

                                        // {"try-hard", 0, NULL, 0},
                                        {NULL, 0, NULL, 0}};

  if (argc == 1) {
    printf("You ran this program with no extra options,\n"
           "maybe it was intentional but you might have \n"
           "more fun trying the available options, run:\n"
           "sssnake -h\n ");
  }

  while ((op = getopt_long(argc, argv, ":aSfs:j:hAzl:m:tx:y:1:", long_options,
                           NULL)) != -1) {
    switch (op) {
    case 'A':
      selectedStyle = ASCII;
      printf("The -A / --ascii flag will be depricated at some point \n"
             "Use \"-l ascii\" instead \n");
      // ascii = 1;
      break;
    case 'a':
      selectedMode = AUTOPILOT;
      printf("The -a / --autopilot flag will be depricated at some point \n"
             "Use \"-m autopilot\" instead \n");
      break;
    case 'S':
      selectedMode = SCREENSAVER;
      printf("The -S / --screensaver flag will be depricated at some point \n"
             "Use \"-m screensaver\" instead \n");
      break;
    case 'f':
      selectedStyle = FANCY;
      printf("The -f / --fancy flag will be depricated at some point \n"
             "The fancy mode is now the default look. So there is no need "
             "include this flag \n");
      // fancy = 1;
      break;
    case 'm':
      if (strcmp(optarg, "arcade") == 0)
        selectedMode = ARCADE;
      else if (strcmp(optarg, "autopilot") == 0)
        selectedMode = AUTOPILOT;
      else if (strcmp(optarg, "screensaver") == 0)
        selectedMode = SCREENSAVER;
      else if (strcmp(optarg, "normal") == 0)
        selectedMode = NORMAL;
      else {
        printf("Incomplete or invalid argument for -m / --mode\n");
        exit(0);
      }
      break;
    case 'l':
      if (strcmp(optarg, "ascii") == 0)
        selectedStyle = ASCII;
      else if (strcmp(optarg, "full") == 0)
        selectedStyle = FULL;
      else if (strcmp(optarg, "fancy") == 0)
        selectedStyle = FANCY;
      else if (strcmp(optarg, "dots") == 0)
        selectedStyle = DOTS;
      else {
        printf("Incomplete or invalid argument for -l / --look\n");
        exit(0);
      }
      break;

    case 'j':
      junk = atoi(optarg);
      if (junk <= 0 || junk > 10)
        junk = 0;
      break;
    case 's':
      speedMult = 21 - atoi(optarg);
      if (speedMult > 0 && speedMult <= 20) {
        speed = 10000 * speedMult;
      }
      break;
    case 'z':
      score = 1;
      break;
    case 't':
      teleport = 1;
      break;
    case 'x':
      maxX = atoi(optarg);
      if (maxX < 5) {
        printf("Minimum value of x supported is 5. \n");
        exit(0);
      }
      break;
    case 'y':
      maxY = atoi(optarg);
      if (maxY < 5) {
        printf("Minimum value of y supported is 5. \n");
        exit(0);
      }
      break;
    case 1:

      selectedAlgorithm = atoi(optarg);
      if (selectedAlgorithm > 2 || selectedAlgorithm < 1) {
        printf("Invalid algorithm!! \n");
        printf("Available algorithms:\n");
        printf("try-hard 1 for greedy 1.\n");
        printf("try-hard 2 for greedy 2.\n");

        exit(0);
      }
      break;
    case 'h':
      print_help();
      exit(0);
    case ':':
      printf("option needs a value\n");
      exit(0);
    case '?':
      printf("unknown option: %c\n", optopt);
      exit(0);
    }
  }
}

void print_help() {

  printf(
      "Usage: sssnake [OPTIONS]\n"
      "Options:\n"

      "  -m op, --mode=op     Mode in which the program will run. The "
      "available "
      "modes are:\n"
      "                     normal, arcade, autopilot and "
      "screensaver.(Default: normal) \n"
      "  -l op, --look=op     Style in of the cells in the game. The available "
      "styles are:\n"
      "                     fancy, full, ascii and dots .(Default: "
      "fancy) \n"
      //"  -a, --autopilot    The game plays itself. (Default: no)\n"
      //"  -A, --ascii        Use ascii characters. (Default: no)\n"
      "  -s, --speed=N      Speed of the game, from 1 to 20. (Default: 1 )\n"
      //"  -S, --screensaver  Autopilot, but it restarts when it dies. (Default:
      //"
      //"no)\n"
      "  -j, --junk=N       Add random blocks of junk, levels from 1 to 5. "
      "(Default: 0 )\n"
      "  -x N, --maxX=N     Define the width of the game field.\n"
      "                     A single unit equals to two terminal columns\n"
      "  -y N, --maxY=N     Define the height of the game field.\n"
      "                     A single unit equals to one terminal row\n"
      "  -z, --score        Shows the size of the snake at any time.\n"
      //"  -f, --fancy        Add a fancy spacing between blocks. (Default:
      // no)\n"
      "  -t, --teleport     Teleport between borders.\n"
      "  -h, --help         Print help message. \n"
      "  --try-hard N       Makes the snake (almost) unkillable in the "
      "autopilot/screensaver mode\n."

      "                     For now there are two options (algorithms):\n"
      "                     \"--try-hard 1\" is cpu efficient, good for big "
      "boards.\n"
      "                     \"--try-hard 2\" uses more cpu, it reaches the "
      "food faster and produces a cleaner board.\n"
      "                     Neither of the two works well with junk in the "
      "board.\n"

      "Try to run something like this :\n"
      "sssnake -s 15 -j 5 -m screensaver\n"

      "For bugs or new features go to : "
      "https://github.com/AngelJumbo/sssnake\n");
}
