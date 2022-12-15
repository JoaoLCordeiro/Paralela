#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void TrataEntradas (int argc, char* argv[], int* nmsg, int* tmsg, int* npro, int* bloq){
	if ((argc != 5) && (argc != 4)){
		fprintf(stderr, "A chamada nao possui o numero correto de argumentos :(\n");
		exit(1);
	}

	*nmsg = atoi(argv[1]);
	if (*nmsg % 2 != 0){
		fprintf(stderr, "Tamanho das mensagens inválido (%d não é múltiplo de 2)\n", *nmsg);
		exit(1);
	}

	*tmsg = atoi(argv[2]);
	if (*tmsg % 8 != 0){
		fprintf(stderr, "Tamanho das mensagens inválido (%d não é múltiplo de 8)\n", *tmsg);
		exit(1);
	}

	*npro = atoi(argv[3]);
	if (*npro != 2){
		fprintf(stderr, "Número de processos MPI inválido (%d é diferente de 2)\n", *npro);
		exit(1);
	}

	*bloq = 1;	//DEFAULT

	if (argc == 5){
		if (strcmp(argv[4], "-nbl") == 0)
			*bloq = 0;
	}

	fprintf(stderr, "nmsg = %d\ntmsg = %d\nnpro = %d\nbloq = %d\n", *nmsg, *tmsg, *npro, *bloq);
}

int main (int argc, char* argv[]){
	int nmsg, tmsg, npro, bloq;
	TrataEntradas (argc, argv, &nmsg, &tmsg, &npro, &bloq);
	//passando daqui, temos certeza que as entradas são válidas

	MPI_Init( &argc, &argv);

	return 0;
}