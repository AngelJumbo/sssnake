PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man

sssnake: main.c autopilot.c xymap.c structs.c snake.c draw.c 
	$(CC) -w $(ncursesw5-config --cflags --libs) main.c autopilot.c xymap.c structs.c snake.c draw.c -lncursesw -o sssnake

debug: main.c autopilot.c xymap.c structs.c snake.c draw.c 
	$(CC) -w $(ncursesw5-config --cflags --libs) -g main.c autopilot.c xymap.c structs.c snake.c draw.c -lncursesw -o sssnake


.PHONY: genman
genman:
	pandoc ./docs/sssnake.1.md -s -t man -o ./docs/sssnake.1

.PHONY: clean
clean:
	$(RM) sssnake

.PHONY: install
install: sssnake
	install					-D sssnake 					$(DESTDIR)$(PREFIX)/bin/sssnake
	install	-m 644	-D ./docs/sssnake.1	$(DESTDIR)$(MANDIR)/man1/sssnake.1

.PHONY: uninstall
uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/sssnake
	$(RM) $(DESTDIR)$(MANDIR)/man1/sssnake.1

