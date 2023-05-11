CC = gcc
FLAGS = -Wall -g
TARGETS = client server # stnc

.PHONY: all clean

all: $(TARGETS)

# stnc: stnc.c stnc.h
# 	$(CC) $(FLAGS) -c $^
# 	$(CC) $(FLAGS) -o $@ $@.o

server: server.c chat.h
	$(CC) $(FLAGS) -c $^
	$(CC) $(FLAGS) -o $@ $@.o

client: client.c chat.h
	$(CC) $(FLAGS) -c $^
	$(CC) $(FLAGS) -o $@ $@.o


clean:
	rm -f *.o *.h.gch $(TARGETS)
