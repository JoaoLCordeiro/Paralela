CFLAGS = -Wall
OBJS = trabalho1.o

all: trabalho1

trabalho1: $(OBJS)
	gcc -o prefixSumPth $(CFLAGS) $(OBJS)

trabalho1.o: trabalho1.c
	gcc $(CFLAGS) -c trabalho1.c

clean:
	-rm -f *~ *.o