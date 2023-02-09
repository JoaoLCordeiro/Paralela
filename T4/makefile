CFLAGS = -Wall -g
OBJSA = trabalho4-a.o
OBJSB = trabalho4-b.o
OBJSC = trabalho4-c.o
OBJSD = trabalho4-d.o

all: trabalho4-a trabalho4-b trabalho4-c trabalho4-d

trabalho4-a: $(OBJSA)
	mpic++ -o trabalho4-a $(CFLAGS) $(OBJSA)

trabalho4-b: $(OBJSB)
	mpic++ -o trabalho4-b $(CFLAGS) $(OBJSB)

trabalho4-c: $(OBJSC)
	mpic++ -o trabalho4-c $(CFLAGS) $(OBJSC)

trabalho4-d: $(OBJSD)
	mpic++ -o trabalho4-d $(CFLAGS) $(OBJSD)

trabalho4-a.o: trabalho4-a.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4-a.c

trabalho4-b.o: trabalho4-b.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4-b.c

trabalho4-c.o: trabalho4-c.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4-c.c

trabalho4-d.o: trabalho4-d.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4-d.c

clean:
	-rm -f *~ *.o