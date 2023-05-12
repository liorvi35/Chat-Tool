CC = gcc
FLAGS = -Wall -g

.PHONY: all default clean chat test


default: all

all: chat test stnc

stnc: stnc.o chatClient.o chatServer.o testClient.o testServer.o
	$(CC) $(FLAGS) -o $@ $^

stnc.o: stnc.c
	$(CC) $(FLAGS) -c $^

chat: chatClient.c chatServer.c
	$(CC) $(FLAGS) -c chatClient.c
	$(CC) $(FLAGS) -c chatServer.c

test: testClient.c testServer.c
	$(CC) $(FLAGS) -c testClient.c
	$(CC) $(FLAGS) -c testServer.c

clean:
	rm -f *.o *.h.gch stnc
