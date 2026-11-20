# Epic RPG Adventure
#   make       -> ./rpg (classic presentation)
#   make tui   -> ./rpg-tui and ./elderbrook-tui (ASCII/TUI presentation)
#   make clean -> remove generated binaries and objects

CC      = gcc
CFLAGS  = -std=gnu99 -Wall -Wextra -O2
LDFLAGS =
LIBS    =

SRCS = $(filter-out %_standalone.c,$(wildcard *.c))
GAME_SRCS = $(filter-out rpg.c,$(SRCS))
CLASSIC_OBJS = $(GAME_SRCS:.c=.o)
TUI_OBJS = $(GAME_SRCS:.c=.tui.o)

.PHONY: all tui clean

all: rpg

rpg: $(CLASSIC_OBJS) rpg.o
	$(CC) $(CFLAGS) -o $@ $(CLASSIC_OBJS) rpg.o $(LDFLAGS) $(LIBS)

rpg-tui: $(TUI_OBJS) rpg_tui.o
	$(CC) $(CFLAGS) -o $@ $(TUI_OBJS) rpg_tui.o $(LDFLAGS) $(LIBS)

elderbrook-tui: rpg-tui
	ln -sf rpg-tui $@

tui: rpg-tui elderbrook-tui
	@echo "Built rpg-tui and elderbrook-tui — run with: ./elderbrook-tui"

rpg.o: rpg.c
	$(CC) $(CFLAGS) -c -o $@ $<

rpg_tui.o: rpg.c
	$(CC) $(CFLAGS) -DRPG_TUI -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.tui.o: %.c
	$(CC) $(CFLAGS) -DRPG_TUI -c -o $@ $<

clean:
	rm -f *.o *.tui.o rpg rpg-tui elderbrook-tui
