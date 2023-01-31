CFLAGS = -Wall -g
OBJSM = trabalho4-m.o
OBJSB = trabalho4-b.o

all: trabalho4-m trabalho4-b

trabalho4-b: $(OBJSB)
	mpic++ -o trabalho4-b $(CFLAGS) $(OBJSB)

trabalho4-m: $(OBJSM)
	mpic++ -o trabalho4-m $(CFLAGS) $(OBJSM)

trabalho4-b.o: trabalho4-b.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4-b.c

trabalho4-m.o: trabalho4-m.c chrono.c
	mpic++ $(CFLAGS) -c trabalho4-m.c

clean:
	-rm -f *~ *.o