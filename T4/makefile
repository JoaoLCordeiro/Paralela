CFLAGS = -Wall -g
OBJS = trabalho4.o

all: trabalho4

trabalho4: $(OBJS)
	mpic++ -o trabalho4 $(CFLAGS) $(OBJS)

trabalho4.o: trabalho4.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4.c

clean:
	-rm -f *~ *.o