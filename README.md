# sssnake(smart and sexy snake)
The classic snake game for the terminal that can plays itself and be use like a screensaver.
[![asciicast](https://asciinema.org/a/477685.svg)](https://asciinema.org/a/477685)
## Installation


```
make
make install
```

ncursesw is required.
(depending of the distro this might be included in the standard ncurses library)

## Usage

By default it will run like a normal snake game that you can control with wasd, hjkl(vi keys) or the arrow keys.
But try this

```
sssnake -f -S -s 15
```

or

``` 
sssnake -S -s 15 -j 5
```
press Q to quit. 
Use the -h options to see the option details.
 

## Contributing

Any improvements are welcome especially in the pathfinding algorithm because now its a basic A\* implementation and sometimes the snake traps itselt.

## Credits

- Min heap implementation base on Martin Broadhurst min-heap ( http://www.martinbroadhurst.com/min-heap-in-c.html ) 
- A\* base on (https://www.geeksforgeeks.org/a-search-algorithm/)
