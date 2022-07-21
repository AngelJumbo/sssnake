PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man

sssnake: main.c autopilot.c xymap.c essentials.c snake.c 
	$(CC) -w $(ncursesw5-config --cflags --libs) main.c autopilot.c xymap.c essentials.c snake.c -lncursesw -o sssnake
#autopilot.o: autopilot.c xymap.c essentials.c snake.c 
#	gcc -c autopilot.c xymap.c essentials.c snake.c

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

