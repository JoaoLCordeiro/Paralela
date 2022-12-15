#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TYPEMSG long int

int nProc, rankProc;

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

TYPEMSG* CriaMsg (int numIndices){
	//alloca o vetor buffer para a msg
	TYPEMSG* buffMsg = (TYPEMSG*) malloc (numIndices*sizeof(TYPEMSG));

	int comeco,fim;

	comeco = numIndices * rankProc + 1;
	fim = (1 + rankProc) * numIndices;

	//for que funciona tanto para rank == 1 e rank == 0
	for (int i = comeco ; i <= fim ; i++)
		buffMsg[i-comeco] = i;
	
	return buffMsg;
}

int main (int argc, char* argv[]){
	int nmsg, tmsg, npro, bloq;
	TrataEntradas (argc, argv, &nmsg, &tmsg, &npro, &bloq);
	//passando daqui, temos certeza que as entradas são válidas

	//inicializa e guarda quantos processos existem e o rank do processo atual
	MPI_Init( &argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &nProc);
	MPI_Comm_rank (MPI_COMM_WORLD, &rankProc);

	fprintf (stderr, "Passou a parte que pega o rank e o size\n");

	int ni	 = tmsg/sizeof(TYPEMSG);
	int otherProc = (rankProc + 1) % 2;
	TYPEMSG* buffMsgSend = CriaMsg(ni);
	TYPEMSG* buffMsgRecv = (TYPEMSG*) malloc (tmsg);
	MPI_Status statusRecv;

	fprintf (stderr, "Passou o cria msg e o malloc\n");

	//é bloqueante ou não
	if (bloq == 1)
		if (rankProc == 0)
			for (int i_msg = 0 ; i_msg < nmsg ; i_msg++){
				MPI_Ssend (buffMsgSend, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD);
				MPI_Recv (buffMsgRecv, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD, &statusRecv);
			}
		else
			for (int i_msg = 0 ; i_msg < nmsg ; i_msg++){
				MPI_Recv (buffMsgRecv, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD, &statusRecv);
				MPI_Ssend (buffMsgSend, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD);
			}
	else
		for (int i_msg = 0 ; i_msg < nmsg ; i_msg++){
			MPI_Send (buffMsgSend, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD);
			MPI_Recv (buffMsgRecv, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD, &statusRecv);
		}

	fprintf (stdout, "Processo %d:	vetorSend[0] = %ld	vetorRecv[0] = %ld\n", rankProc, buffMsgSend[0], buffMsgRecv[0]);

	fprintf (stderr, "Passou a troca de mensagens\n");

	MPI_Finalize();

	return 0;
}