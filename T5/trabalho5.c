#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <mpi.h>

#include "chrono.c"
#include "verificaKNN.c"

typedef struct par
{
	float	distancia;
	int		ponto;
} t_par;

MPI_Datatype	MPI_T_PAR;

int nProc, rank;
int nQ, nP, nD, k;
float* Q;
float* P;
int* R;

chronometer_t cronometro;

//função que imprime as respostas na tela
void imprimeResposta(){
	fprintf(stdout, "\n");
	for (int i = 0 ; i < nQ ; i++){
		fprintf(stdout, "Respostas para a linha %d\n\n", i);

		for (int j = 0 ; j < k ; j++){
			fprintf(stdout, "Ponto:	%d\n", R[i*k + j]);
		}

		fprintf(stdout, "\n");
	}
}

//função de debug: cria arquivos com dados desejados
void geraArquivoMatrizes(t_par* menores){
	char* num	  = (char *) malloc (2*sizeof(char));
	sprintf(num, "%d", rank);

	//matriz P
	/*char* nomearqP = (char *) malloc (16*sizeof(char));

	strcpy(nomearqP, "matrizP-rank");
	strcat(nomearqP, num);
	strcat(nomearqP, ".txt");

	FILE* filematrizP = fopen (nomearqP,"w+");

	for (int i = 0 ; i < nP ; i++){
		for (int j = 0 ; j < nD ; j++){
			fprintf(filematrizP, "%f	", P[i*nD + j]);
		}
		fprintf(filematrizP, "\n");
	}

	fclose(filematrizP);

	//matriz Q
	char* nomearqQ = (char *) malloc (16*sizeof(char));

	strcpy(nomearqQ, "matrizQ-rank");
	strcat(nomearqQ, num);
	strcat(nomearqQ, ".txt");

	FILE* filematrizQ = fopen (nomearqQ,"w+");

	for (int i = 0 ; i < nQ ; i++){
		for (int j = 0 ; j < nD ; j++){
			fprintf(filematrizQ, "%f	", Q[i*nD + j]);
		}
		fprintf(filematrizQ, "\n");
	}

	fclose(filematrizQ);*/

	//menores
	char* nomearqM = (char *) malloc (16*sizeof(char));

	strcpy(nomearqM, "vetorM-rank");
	strcat(nomearqM, num);
	strcat(nomearqM, ".txt");

	FILE* vetorM = fopen (nomearqM,"w+");

	for (int i = 0 ; i < k ; i++){
		fprintf(vetorM, "Ponto:	%d	Distancia:	%f\n", menores[i].ponto ,menores[i].distancia);
	}

	fclose(vetorM);
}

//verifica o número de entradas do programa
void verificaEntradas(int argc, char *argv[]){
	if (argc != 5){
		fprintf(stderr, "Erro:	Número incorreto de argumentos. Tente:\n./trabalho5 <nq> <np> <nD> <k>\n");
		exit(1);
	}
}

//gera um conjunto de dados no ponteiro C com nc elementos de D dimensoes
void geraConjuntoDeDados(float* C, int nc, int D ){
	for (int i = 0 ; i < nc ; i++)
		for (int j = 0 ; j < D ; j++)
			C[i*D + j] = (float) rand();
}

//envia o pedaço da matriz que o processo destino vai usar (da linha "comeco" até a "fim")
void enviaParteMatrizP (int destino, int comeco, int fim){
	for (int i = comeco ; i < fim ; i++)
		MPI_Send (&(P[i*nD]), nD, MPI_FLOAT, destino, 0, MPI_COMM_WORLD);
}

//recebe um pedaço da matriz P (da linha "começo" até a "fim")
void recebeParteMatrizP (int comeco, int fim){
	MPI_Status statusRecv;

	for (int i = comeco ; i < fim ; i++){
		MPI_Recv (&(P[i*nD]), nD, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &statusRecv);
	}
}

//retorna o quadrado do módulo da diferença de a e b
float modDiffSqr (float a, float b){
	float res;
	if (a < b)
		res = b - a;
	else
		res = a - b;

	return res*res;
}

//cria um vetor com as menores distâncias e seus pontos
void adicionaMenor (float dist, int ponto, int comeco, t_par *vetor){
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

//função de debug: imprime o vetor de menores distancias
void imprimeMenores (t_par *menores_dist){
	int tam = k*nProc;

	for (int i = 0 ; i < tam ; i++)
		fprintf (stdout, "%f\n", menores_dist[i].distancia);
}

//função que inicializa o vetor menores_dist com valores gigantes
void encheInf (t_par* menores_dist){
	for (int i = 0 ; i < k ; i++)
		menores_dist[i].distancia = FLT_MAX;
}

//calcula os pontos de P mais próximos do ponto da linhaQ
void calculaPontoQ (int linhaQ, int comecoP, int fimP){
	MPI_Status statusRecv;

	//manda a linha do Q para os outros processos
	if (rank == 0)
		for (int i = 1 ; i < nProc ; i++)
			MPI_Send (&(Q[linhaQ*nD]), nD, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
	else
		MPI_Recv (&(Q[linhaQ*nD]), nD, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &statusRecv);


	//um vetor que vai guardando as menores distancias e seus índices
	t_par *menores_dist;
	if (rank == 0)
		menores_dist = (t_par *) calloc (k*nProc,sizeof(t_par));	//o raiz irá armazenar todas as k menores distâncias dos nProc
	else
		menores_dist = (t_par *) calloc (k,sizeof(t_par));

	//coloca valores grandes inicialmente no vetor
	encheInf (menores_dist);

	//calcula as distâncias e separa as k menores
	for (int i = comecoP ; i < fimP ; i++){
		float distancia = 0;
		
		//calculo da distancia
		for (int j = 0 ; j < nD ; j++){
			distancia += modDiffSqr(Q[linhaQ*nD + j], P[i*nD + j]);
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

	//espera a sincronização de processos
	MPI_Barrier(MPI_COMM_WORLD); 

	//pega apenas os k menores entre todos os processos
	if (rank == 0){
		//a cada iteração, seleciona um ponto
		for (int i = 0 ; i < k ; i++){
			float menor 		= menores_dist[0].distancia;
			int indice_menor	= 0;

			for (int j = 1 ; j < k * nProc ; j++){
				if ((menores_dist[j].distancia != FLT_MAX) && (menores_dist[j].distancia < menor)){
					menor			= menores_dist[j].distancia;
					indice_menor	= j;
				}
			}

			R[linhaQ*k + i]		= menores_dist[indice_menor].ponto;

			//tirando o que pegamos dos candidatos pros proximos menores
			menores_dist[indice_menor].distancia = FLT_MAX;
		}
	}

	//espera a sincronização de processos
	MPI_Barrier(MPI_COMM_WORLD); 
	//a partir daqui, R[linhaQ possui as respostas para a linhaQ]
}

//função que cria e commita o tipo MPI_T_PAR que será usado nos cálculos
void criaMPIPar (){
	const int 			array_of_blocklengths[] 	= {1,1};
	const MPI_Aint 		array_of_displacements[]	= {0,4};
	const MPI_Datatype 	array_of_types[]			= {MPI_FLOAT, MPI_INT};

	if (MPI_Type_create_struct(2, array_of_blocklengths, array_of_displacements, array_of_types, &MPI_T_PAR) != MPI_SUCCESS)
		fprintf(stderr, "Erro:	Não foi possível criar o tipo MPI_T_PAR\n");

	if (MPI_Type_commit(&MPI_T_PAR) != MPI_SUCCESS)
		fprintf(stderr, "Erro:	Não foi possível commitar o tipo MPI_T_PAR\n");
}

//função que calcula o knn
void KNN (float* mQ, int nmQ, float* mP, int nmP, int D, int nk){
	//como todos os argumentos são globais, são passados aqui apenas para adequar à expecificação

	//calcula os k pontos mais próximos do ponto i de Q
	for (int i = 0 ; i < nQ ; i++){
		if (rank == 0)
			fprintf(stdout, "Calculando a linha %d\n", i);
		calculaPontoQ (i, (rank*nP)/nProc, ((rank+1)*nP)/nProc);
	}
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

	Q = (float *) malloc (nQ * nD *sizeof(float));
	P = (float *) calloc (nP * nD, sizeof(float));

	//alocamos a matriz resposta e geramos os conjuntos Q e P
	if (rank == 0){
		R = (int *) malloc (nQ * k * sizeof(int));

		geraConjuntoDeDados(Q, nQ, nD);
		geraConjuntoDeDados(P, nP, nD);
	}

	//espera o rank = 0 gerar as matrizes
	MPI_Barrier(MPI_COMM_WORLD);

	//começa o cronometro
	if (rank == 0){
		chrono_reset(&cronometro);
		chrono_start(&cronometro);
	}

	//dividimos a matriz P em pedaços e mandamos um para cada processo
	if (rank == 0)
		for (int i = 1 ; i < nProc ; i++)
			enviaParteMatrizP (i, (i*nP)/nProc, ((i+1)*nP)/nProc);
	else
		recebeParteMatrizP ((rank*nP)/nProc, ((rank+1)*nP)/nProc);

	MPI_Barrier(MPI_COMM_WORLD);

	//a partir daqui, cada processo possui o seu pedaço da matrizP com qual irá trabalhar

	//calcula o KNN
	KNN(Q, nQ, P, nP, nD, k);

	if (rank == 0){
		chrono_stop(&cronometro);
		double tempo	= (double) chrono_gettotal(&cronometro) / (1000*1000*1000);

		verificaKNN(Q, nQ, P, nP, nD, k, R);

		fprintf (stdout, "\nNP:	%d\n", nProc);
		fprintf (stdout, "\nTempo:	%f\n", tempo);

		imprimeResposta();
	}

	MPI_Finalize();

	return 0;
}