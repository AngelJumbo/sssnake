PREFIX = /usr/local

sssnake: main.c autopilot.c xymap.c essentials.c snake.c 
	gcc main.c autopilot.c xymap.c essentials.c snake.c -lncurses -o sssnake
autopilot.o: autopilot.c xymap.c essentials.c snake.c 
	gcc -c autopilot.c xymap.c essentials.c snake.c

.PHONY: clean
clean:
	rm ./sssnake

.PHONY: install
install: sssnake
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/sssnake

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/sssnake

