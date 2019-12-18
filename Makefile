all : manager.o
	$(CC) -o manager manager.o

manager.o : manager.c
	$(CC) -o manager.o -c manager.c

.PHONY: clean

clean :
	rm -f *.o manager
