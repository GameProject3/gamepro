CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/usr/include/SDL2
LDFLAGS = -lSDL2 -lSDL2_ttf -lm

SRCS = main.c enigme.c
OBJS = $(SRCS:.c=.o)
TARGET = se7en

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

install:
	sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev fonts-dejavu

.PHONY: all clean run install
