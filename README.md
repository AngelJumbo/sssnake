# sssnake
The classic snake game for the terminal that can play itself and be used like a screensaver.
[![asciicast](https://asciinema.org/a/477685.svg)](https://asciinema.org/a/477685)
## Installation

You need ncurses :

```
apt install libncurses-dev       #Debian/Ubuntu/Mint
yum install ncurses-devel        #RHEL/CentOS/Fedora and Rocky Linux/AlmaLinux
pacman -S ncurses                #Arch Linux
zypper install ncurses-devel     #OpenSUSE 
xbps-install -S ncurses-devel    #Void linux
brew install ncurses             #MacOS
```

With that you can just do:


```
git clone https://github.com/AngelJumbo/sssnake.git
cd sssnake
make
sudo make install
```


## Usage

You can check the manpage or use the -h option to see the details of what this program can do.
So instead I would like to show you a few things that I like to run.

```
sssnake -m autopilot -s 15 -j 10
```

[![Watch the video](https://img.youtube.com/vi/qNDcn5tdyno/mqdefault.jpg)](https://youtu.be/qNDcn5tdyno)

```
sssnake -m autopilot -s 15 -z -t -l ascii
```

[![Watch the video](https://img.youtube.com/vi/XTS2CXHzBjA/mqdefault.jpg)](https://youtu.be/XTS2CXHzBjA)


```
sssnake -m screensaver -s 15 -z -x 8 -y 8 --try-hard 1
```

[![Watch the video](https://img.youtube.com/vi/oh4CK8wPU-4/mqdefault.jpg)](https://youtu.be/oh4CK8wPU-4)



## Planned features 

### Definitely be added
  - ~~Pause key~~ Spacebar is now the pause key.
  - ~~Increase and decrease speed keys~~ You can increase or decrease the speed with + and - in autopilot/screensaver modes.
  - ~~An alternative/replacement to the A\* algorithm (probably BFS)~~ BFS is now available with: `--short-path=bfs`.
### May be added
  - Hamiltonian cycles.
  - Custom colors.
  - IDA\* to find the shortest path. 

## Faq

### Can I control the snake?
  Yes, that's the default mode or you can try the arcade mode where the game increases the speed every time you eat.
  The controls are wasd, the arrow keys or hjkl(vim keys).

### Does the snake fills the entire screen/terminal with the autopilot?
  If you use the "--try-hard" options the snake will get pretty close and sometimes it will fill the terminal.
  I implemented two algorithms "--try-hard 1" is good for big terminals/boards. You can test it running:
  ```
  sssnake -m autopilot -s 15 --try-hard 1
  ```

  "--try-hard 2" uses more cpu and it can get laggy with big boards but it has more chances to fill the board. Try:
  ```
  sssnake -m autopilot -s 10 -x 10 -y 10 --try-hard 2
  ```
  Neither of the two algorithms work well with junk.

### Why the name "sssnake"?
   All the snake games in the terminal that I found use ascii characters and lets be honest, they are kinda ugly. 
   I try to do something more visually appealing, something "sexy" and it can play itself so it is "smart". 
   Smart and sexy snake => sssnake.
## A\* vs BFS
   At first this project was just an implementation of A\* in the snake game, after finding [chuyangliu's project](https://github.com/chuyangliu/snake/) I implemented their greedy solver with my A\* instead of their BFS. Noticing that the behavior of the two implementations is different I decided to implement BFS in this project also as an alternative to A\*.
You can see the result here:

[![Watch the video](https://img.youtube.com/vi/I1QOw_iDIB8/hqdefault.jpg)](https://youtu.be/I1QOw_iDIB8)

Commands of the video:
For A\*:
```
sssnake -m autopilot -x 20 -y 20 -s 17 --try-hard 2 -z
```
For BFS:
```
sssnake -m autopilot -x 20 -y 20 -s 17 --try-hard 2 -z --short-path=bfs
```


## Credits

- Min heap implementation base on Martin Broadhurst min-heap ( http://www.martinbroadhurst.com/min-heap-in-c.html ) 
- A\* base on (https://www.geeksforgeeks.org/a-search-algorithm/)
- The two "--try-hard" algorithms that I implemented were inspired by chuyangliu's greedy solver ( https://github.com/chuyangliu/snake/blob/master/docs/algorithms.md#greedy-solver )
