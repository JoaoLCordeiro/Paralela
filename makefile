CFLAGS = -Wall -g
LDFLAGS = -pthread
OBJS = trabalho1.o

all: trabalho1

trabalho1: $(OBJS)
	gcc -o prefixSumPth $(CFLAGS) $(LDFLAGS) $(OBJS)

trabalho1.o: trabalho1.c
	gcc $(CFLAGS) $(LDFLAGS) -c trabalho1.c

clean:
	-rm -f *~ *.o