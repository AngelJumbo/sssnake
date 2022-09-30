---
title: SSSNAKE
section: 1
header: User Manual
footer: Ver. 0.3.0
date: September 29, 2022
---
# NAME
sssnake - Classic snake game on the terminal that can run in autopilot.

# SYNOPSIS
**sssnake** [*OPTION*]...

# DESCRIPTION
**sssnake** is the classic snake game, written in c capable of running on autopilot.

# OPTIONS
**-m MODE, --mode=MODE** 
: Mode in which the program will run. MODE can be:

    * normal (Default mode where you control the snake)
    * arcade (A board with specific dimensions and the speed increases when the snake eats )
    * autopilot (The game plays itself)
    * screensaver (Same as autopilot but the game restarts every time the snake dies)

**-l STYLE, --look=STYLE** 
: Style how the snake, food, walls, and junk look. STYLE can be:

    * fancy (Default mode )
    * full (The snake is made out of full blocks)
    * ascii (Every thing is made out of ascii characters)
    * dots (The snake is made out of little square dots)

**-s NUMBER, --speed=NUMBER** 
: Speed of the game, from 1 to 20. (Default: 1 ).

**-x DIMENSION, --maxX=DIMENSION** 
: Define the width of the board (minimum value of DIMENSION is 5). A single unit is equal to two terminal columns.

**-y DIMENSION, --maxY=DIMENSION** 
: Define the height of the board (minimum value of DIMENSION is 5). A single unit is equal to one terminal row.

**-j NUMBER, --junk=NUMBER** 
: Add random blocks of junk, levels from 1 to 10. (Default: 0 ).

**-z, --score** 
: Shows the size of the snake at any time.

**-t, --teleport** 
: Allows the snake to teleport between borders.

**--try-hard=NUMBER** 
:  Makes the snake unkillable in the autopilot/screensaver mode. For now there are two options (algorithms):

    * "--try-hard 1" is cpu efficient, good for big boards.
    * "--try-hard 2" uses more cpu, it reaches the food faster and produces a cleaner board.

**--short-path=ALGORITHM** 
:  Lets you set the algorithm used to find the shortest path. There is three algorithms available:

    * astar: A\* using a simple heuristic, it tends to make a lot of turns (default).
    * bfs: Breadth-first search, it produces straight paths.
    * asfixed: A\* with a heuristic that puts extra cost to turns, the paths produced are a middle ground between the other two algorithms.

**-h, --help** 
: Print help message..

# CONTROLS

## NORMAL/ARCADE:

**wasd, hjkl, up down left right**
: Basic directional controls.

**spacebar**
: Pause game.

**q**
: Quit game.

## AUTOPILOT/SCREENSAVER:

**+**
: Increases the speed of the game.

**-**
: Decreases the speed of the game.

**spacebar**
: Pause game.

**q**
: Quit game.

## SCREENSAVER:

**Any key except +,- and spacebar**
: Quit game.

# AUTHORS
Written by Angel Jumbo(anarjumb@protonmail.com).

# BUGS
Submit bug reports online at: <https://github.com/AngelJumbo/issues>
