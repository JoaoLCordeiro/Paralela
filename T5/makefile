CFLAGS = -Wall -g
OBJS = trabalho5.o

all: trabalho5

trabalho5: $(OBJS)
	mpic++ -o trabalho5 $(CFLAGS) $(OBJS)

trabalho5.o: trabalho5.c chrono.c
	mpic++ $(CFLAGS) -c trabalho5.c
clean:
	-rm -f *~ *.o