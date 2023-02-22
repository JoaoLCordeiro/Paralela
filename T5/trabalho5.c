#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <mpi.h>

typedef struct par
{
	float	distancia;
	int		ponto;
} t_par;

MPI_Datatype	MPI_T_PAR;

int nProc, rank;
int nQ, nP, nD, k;
float** Q;
float** P;
t_par** R;


void imprimeResposta(){
	fprintf(stdout, "\n");
	for (int i = 0 ; i < nQ ; i++){
		fprintf(stdout, "Respostas para a linha %d\n\n", i);

		for (int j = 0 ; j < k ; j++){
			fprintf(stdout, "Ponto:	%d	Distancia:	%f\n", R[i][j].ponto, R[i][j].distancia);
		}

		fprintf(stdout, "\n");
	}
}

void geraArquivoMatrizes(){
	char* nomearqP = (char *) malloc (16*sizeof(char));

	strcpy(nomearqP, "matrizP-rank");
	char* num	  = (char *) malloc (2*sizeof(char));
	sprintf(num, "%d", rank);
	strcat(nomearqP, num);
	strcat(nomearqP, ".txt");

	FILE* filematrizP = fopen (nomearqP,"w+");

	for (int i = 0 ; i < nP ; i++){
		for (int j = 0 ; j < nD ; j++){
			fprintf(filematrizP, "%f	", P[i][j]);
		}
		fprintf(filematrizP, "\n");
	}

	fclose(filematrizP);

	char* nomearqQ = (char *) malloc (16*sizeof(char));

	strcpy(nomearqQ, "matrizQ-rank");
	strcat(nomearqQ, num);
	strcat(nomearqQ, ".txt");

	FILE* filematrizQ = fopen (nomearqQ,"w+");

	for (int i = 0 ; i < nQ ; i++){
		for (int j = 0 ; j < nD ; j++){
			fprintf(filematrizQ, "%f	", Q[i][j]);
		}
		fprintf(filematrizQ, "\n");
	}

	fclose(filematrizQ);
}

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

float modDiffSqr (float a, float b){
	float res;
	if (a < b)
		res = b - a;
	else
		res = a - b;

	return res*res;
}

void adicionaMenor (float dist, int ponto, int comeco, t_par *vetor){
	if (ponto - comeco < k){
		vetor[ponto-comeco].distancia	= dist;
		vetor[ponto-comeco].ponto		= ponto;
	}

	int i = -1;
	while ((i+1 < k) && (dist < vetor[i+1].distancia)){
		i++;
		vetor[i].distancia	= vetor[i+1].distancia;
		vetor[i].ponto		= vetor[i+1].ponto;
	}

	if (i >= 0){
		vetor[i].distancia	= dist;
		vetor[i].ponto		= ponto;
	}	
}

void imprimeMenores (t_par *menores_dist){
	int tam = k*nProc;

	for (int i = 0 ; i < tam ; i++)
		fprintf (stdout, "%f\n", menores_dist[i].distancia);
}

void calculaPontoQ (int linhaQ, int comecoP, int fimP){
	MPI_Status statusRecv;

	//manda a linha do Q para os outros processos
	if (rank == 0)
		for (int i = 1 ; i < nProc ; i++)
			MPI_Send (Q[linhaQ], nD, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
	else
		MPI_Recv (Q[linhaQ], nD, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &statusRecv);


	//um vetor que vai guardando as menores distancias e seus índices
	t_par *menores_dist;
	if (rank == 0)
		menores_dist = (t_par *) calloc (k*nProc,sizeof(t_par));
	else
		menores_dist = (t_par *) calloc (k,sizeof(t_par));

	for (int i = comecoP ; i < fimP ; i++){
		float distancia = 0;
		
		//calculo da distancia
		for (int j = 0 ; j < k ; j++){
			distancia += modDiffSqr(Q[linhaQ][j], P[i][j]);
		}

		adicionaMenor (distancia, i, comecoP, menores_dist);
	}
	//depois daqui, temos um vetor de tamanho k com os pontos de P mais proximos
	
	//compartilhar com o processo raíz os pontos mais proximos do ponto Q
	if (rank == 0){
		for (int i = 1 ; i < nProc ; i++)
			MPI_Recv (&menores_dist[i*k], k, MPI_T_PAR, i, 0, MPI_COMM_WORLD, &statusRecv);
	}
	else{
		MPI_Send (menores_dist, k, MPI_T_PAR, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD); 

	//pega apenas os k menores
	if (rank == 0){
		for (int i = 0 ; i < k ; i++){
			float menor 		= menores_dist[0].distancia;
			int indice_menor	= 0;

			for (int j = 1 ; j < k * nProc ; j++){
				if (menores_dist[j].distancia < menor){
					menor			= menores_dist[j].distancia;
					indice_menor	= j;
				}
			}

			R[linhaQ][i].distancia	= menor;
			R[linhaQ][i].ponto		= menores_dist[indice_menor].ponto;

			//tirando o que pegamos dos candidatos pros proximos menores
			menores_dist[indice_menor].distancia = FLT_MAX;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD); 
	//a partir daqui, R[linhaQ possui as resposta]

	//printa na tela para o ponto linhaQ
}

void criaMPIPar (){
	const int 			array_of_blocklengths[] 	= {1,1};
	const MPI_Aint 		array_of_displacements[]	= {0,4};
	const MPI_Datatype 	array_of_types[]			= {MPI_FLOAT, MPI_INT};

	if (MPI_Type_create_struct(2, array_of_blocklengths, array_of_displacements, array_of_types, &MPI_T_PAR) != MPI_SUCCESS)
		fprintf(stderr, "Erro:	Não foi possível criar o tipo MPI_T_PAR\n");

	if (MPI_Type_commit(&MPI_T_PAR) != MPI_SUCCESS)
		fprintf(stderr, "Erro:	Não foi possível commitar o tipo MPI_T_PAR\n");
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

	criaMPIPar();

	Q = (float **) malloc (nQ * sizeof(float *));
	P = (float **) malloc (nP * sizeof(float *));

	if (rank == 0){
		R = (t_par **) malloc (nQ * sizeof(t_par *));
		for (int i = 0 ; i < nQ ; i++)
			R[i] = (t_par *) malloc (k * sizeof(t_par));
	}

	for (int i = 0 ; i < nQ ; i++)
		Q[i] = (float *) malloc (nD * sizeof(float));
	for (int i = 0 ; i < nP ; i++)
		P[i] = (float *) calloc (nD , sizeof(float));

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

	for (int i = 0 ; i < nQ ; i++){
		if (rank == 0)
			fprintf(stdout, "Calculando a linha %d\n", i);
		calculaPontoQ (i, (rank*nP)/nProc, ((rank+1)*nP)/nProc);
	}

	if (rank == 0)
		imprimeResposta();

	MPI_Finalize();

	return 0;
}