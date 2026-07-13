PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man

sssnake: main.c autopilot.c xymap.c structs.c snake.c draw.c
	$(CC) -w main.c autopilot.c xymap.c structs.c snake.c draw.c -o sssnake

debug: main.c autopilot.c xymap.c structs.c snake.c draw.c
	$(CC) -w -Wall -g main.c autopilot.c xymap.c structs.c snake.c draw.c -o sssnake

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
