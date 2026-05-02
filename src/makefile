CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -I.
LIBS    = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
TARGET  = ../jeu

SRCS = main.c joueur.c back.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) ../jeu

.PHONY: all clean
