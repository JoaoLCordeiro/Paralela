CFLAGS = -Wall -g
OBJS = trabalho3.o

all: trabalho3

trabalho3: $(OBJS)
	mpicc -o trabalho3 $(CFLAGS) $(OBJS)

trabalho3.o: trabalho3.c
	mpicc $(CFLAGS) -c trabalho3.c

clean:
	-rm -f *~ *.o