installPath = /usr/local/bin

CC		= cc
LIBS	= -lX11
CFLAGS = -g -std=gnu17 -pedantic -Wall -O0 -fsanitize=address $(LIBS)
SRC = main.c

options:
	@echo "BUILD OPTIONS"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "LIBS = $(LIBS)"
	@echo "SRC = $(SRC)"
	@echo "installPath = $(installPath)"


build: options
	$(CC) $(CFLAGS) -o statusblocks $(SRC)

debug: options
	$(CC) $(CFLAGS) -DDEBUG -o statusblocks $(SRC)

install: build
	@echo "Installing statusblocks to $(installPath)"	
	install -m 0755 statusblocks $(installPath)
	@echo "Done!"

clean:
	@echo "Cleaning up..."
	rm -f statusblocks