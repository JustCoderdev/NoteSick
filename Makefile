# JustCoderdev Makefile for C projects v6

CORE_FILES =

CC = gcc
CCFLAGS = -xc -std=c89 -ansi -pedantic-errors -pedantic \
		 -Wall -Wextra -Werror -Wshadow -Wpointer-arith \
		 -Wcast-qual -Wcast-align -Wstrict-prototypes \
		 -Wmissing-prototypes -Wconversion -g

IFLAGS = -I./ -I./lib/include -I../include
LDFLAGS = -L./

DFLAGS = -DDEBUG_ENABLE=1
FLAGS = $(CCFLAGS) $(IFLAGS) $(LDFLAGS) $(DFLAGS)

.PHONY: all
.PHONY: bin
.PHONY: clean

all: ytseeker/ytseeker

bin/ytseeker: bin ytseeker/ytseeker.c
	@echo "Compiling... (ytseeker)"
	$(CC) $(FLAGS) ytseeker/ytseeker.c -lssl $(CORE_FILES) -o bin/ytseeker

bin:
	@mkdir -p bin

clean:
	@echo -n "Cleaning... "
	@rm -rf ./bin
	@echo "Done"
