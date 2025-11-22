PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man
NCURSES_FLAGS = $(shell ncursesw5-config --cflags --libs 2>/dev/null)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    ifneq ($(wildcard /opt/homebrew/opt/ncurses),)
        HB_PATH = /opt/homebrew/opt/ncurses
    else
        HB_PATH = /usr/local/opt/ncurses
    endif
    NCURSES_FLAGS = -I$(HB_PATH)/include -L$(HB_PATH)/lib -D_XOPEN_SOURCE_EXTENDED
endif

sssnake: main.c autopilot.c xymap.c structs.c snake.c draw.c 
	$(CC) -w $(NCURSES_FLAGS) main.c autopilot.c xymap.c structs.c snake.c draw.c -lncursesw -o sssnake

debug: main.c autopilot.c xymap.c structs.c snake.c draw.c 
	$(CC) -w $(NCURSES_FLAGS) -Wall -g main.c autopilot.c xymap.c structs.c snake.c draw.c -lncursesw -o sssnake

.PHONY: genman
genman:
	pandoc ./docs/sssnake.1.md -s -t man -o ./docs/sssnake.1

.PHONY: clean
clean:
	$(RM) sssnake

.PHONY: install
install: sssnake
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	install -m 755 sssnake $(DESTDIR)$(PREFIX)/bin/sssnake
	install -m 644 ./docs/sssnake.1 $(DESTDIR)$(MANDIR)/man1/sssnake.1

.PHONY: uninstall
uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/sssnake
	$(RM) $(DESTDIR)$(MANDIR)/man1/sssnake.1
