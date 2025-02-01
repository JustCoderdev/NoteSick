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
.PHONY: ytseeker
.PHONY: clean

all: ytseeker

ytseeker:
	@echo "Compiling... (ytseeker)"
	@mkdir -p bin
	$(CC) $(FLAGS) ytseeker/ytseeker.c $(CORE_FILES) -o bin/ytseeker


clean:
	@echo "Cleaning..."
	@rm -rf ./build
