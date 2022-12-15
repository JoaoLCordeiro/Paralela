MPICXX=mpicpc
MPICC=mpicc
OBJS=printarr_par.o
CFLAGS= -g
BINS=stencil_mpi_ddt_funneled

all: $(BINS)

%.o: %.c Makefile
	$(MPICC) $(CFLAGS) $< -c -lm -o $@

stencil_mpi_ddt_funneled: stencil_mpi_ddt_funneled.c Makefile $(OBJS)
	$(MPICC) $(CFLAGS) -qopenmp -o $@ $< $(OBJS) -lm

clean:
	rm -f $(BINS) $(OBJS)
	rm -fr output*.bmp *.dSYM
