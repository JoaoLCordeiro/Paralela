#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int nProc, rank;
int nQ, nP, nD, k;
float** Q, P;

//verifica o número de entradas do programa
void verificaEntradas(int argc, char *argv[]){
	if (argc != 5){
		fprintf(stderr, "Erro:	Número incorreto de argumentos. Tente:\n./trabalho5 <nq> <np> <nD> <k>\n");
		exit(1);
	}
}

//gera um conjunto de dados no ponteiro C com nc elementos de D dimensoes
void geraConjuntoDeDados(float** C, int nc, int D ){
	for (int i = 0 ; i < nc ; i++)
		for (int j = 0 ; j < D ; j++)
			C[i][j] = (float) rand();
}

//envia o pedaço da matriz que o processo destino vai usar
void enviaParteMatrizP (int destino, int comeco, int fim){
	for (int i = comeco ; i < fim ; i++)
		MPI_Send (P[i], nD, MPI_FLOAT, destino, 0, MPI_COMM_WORLD);
}

void recebeParteMatrizP (int comeco, int fim){
	MPI_Status statusRecv;

	for (int i = comeco ; i < fim ; i++){
		MPI_Recv (P[i], nD, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &statusRecv);
	}
}

void calculaPontoQ (int linhaQ, int comecoP, int fimP){
	//manda a linha do Q para os outros processos
	if (rank == 0)
		for (int i = 1 ; i < nProc ; i++)
			MPI_Send (Q[linhaQ], nD, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
	else{
		MPI_Status statusRecv;
		MPI_Recv (Q[linhaQ], nD, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &statusRecv);
	}

	//um vetor que vai guardando as maiores distancias e seus índices
	//fazer uma struct par (distancia, ponto)?
	
	//compartilhar com o processo raíz os pontos mais proximos do ponto Q

	//o processo raiz pega apenas os maiores de todos os vetores de pares

	//printa na tela para o ponto linhaQ
}

int main (int argc, char *argv[]){

	verificaEntradas(argc, argv);
	nQ	= atoi(argv[1]);
	nP	= atoi(argv[2]);
	nD	= atoi(argv[3]);
	k	= atoi(argv[4]);

	//inicializando o gerador de números aleatórios
	srand(777);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Q = (float **) malloc (nQ * sizeof(float *));
	P = (float **) malloc (nP * sizeof(float *));

	for (int i = 0 ; i < nQ ; i++)
		Q[i] = (float *) malloc (D * sieof(float));
	for (int i = 0 ; i < nP ; i++)
		P[i] = (float *) malloc (D * sieof(float));

	if (rank == 0){
		geraConjuntoDeDados(Q, nQ, nD);
		geraConjuntoDeDados(P, nP, nD);
	}

	//espera o rank = 0 gerar as matrizes
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
		for (int i = 1 ; i < nProc ; i++)
			enviaParteMatrizP (i, (i*nP)/nProc, ((i+1)*nP)/nProc);
	else
		recebeParteMatrizP ((rank*nP)/nProc, ((rank+1)*nP)/nProc);

	MPI_Barrier(MPI_COMM_WORLD);

	//a partir daqui, cada processo possui o seu pedaço da matrizP com qual irá trabalhar

	for (int i = 0 ; i < nQ ; i++)
		calculaPontoQ (i, (rank*nP)/nProc, ((rank+1)*nP)/nProc);

	return 0;
}