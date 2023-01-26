CFLAGS = -Wall -g
OBJS = trabalho3.o

all: trabalho3

trabalho3: $(OBJS)
	mpic++ -o trabalho3 $(CFLAGS) $(OBJS)

trabalho3.o: trabalho3.c chrono.c
	mpic++ $(CFLAGS) -c trabalho3.c

clean:
	-rm -f *~ *.o
