CC = gcc
FLAGS = -Wall -g
TARGETS = stnc

.PHONY: default all clean

default: all

all: $(TARGETS)

stnc: stnc.c stnc.h
	$(CC) $(FLAGS) -c $^
	$(CC) $(FLAGS) -o $@ $@.o

clean:
	rm -f *.o *.h.gch $(TARGETS)
