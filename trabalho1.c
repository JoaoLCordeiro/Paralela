#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "chrono.c"

//#define DEBUGPRINT
#define MAX_THREADS 64

pthread_t vThread[MAX_THREADS];
pthread_barrier_t barreiraThread;

void imprimeVetor (long int *vetor, int n){
	fprintf(stdout, "Vetor:	");
	for (int i = 0 ; i < n ; i++)
		fprintf(stdout, " %ld", vetor[i]);
	fprintf(stdout, "\n");
}

void sumMax (long int* vMax, int elementos){
	for (int i = 1 ; i < elementos ; i++)
		vMax[i] += vMax[i-1];
}

void PrefixSumPart (long int* argum){

	//coloca os argumentos em variáveis
	long int	comeco	= argum[0];
	long int	fim		= argum[1];
	long int*	vetorE	= (long int*) argum[2];
	long int*	vetorS	= (long int*) argum[3];

	fprintf (stdout, "Thread com comeco em %ld e fim em %ld\n", comeco, fim);

	//faz a soma de prefixos na area desejada
	vetorS[comeco] = vetorE[comeco];
	for (int i = comeco+1 ; i < fim ; i++){
		vetorS[i] = vetorE[i] + vetorS[i-1];
	}

	//coloca o maximo nessa posicao
	*((long int *) argum[4]) = vetorS[fim-1];

	//no fim, libera o vetor de argumentos
	free (argum);

	pthread_barrier_wait(&barreiraThread);
}

void sumMaxPrefix (long int* argum){
	//coloca os argumentos em variáveis
	long int	comeco	= argum[0];
	long int	fim		= argum[1];
	long int	thread	= argum[2];
	long int*	vetorS	= (long int*) argum[3];
	long int*	vMax	= (long int*) argum[4];

	if (thread != 0)
		for (int posicao = comeco ; posicao < fim ; posicao++)
			vetorS[posicao] += vMax[thread-1];

	free(argum);
	
	pthread_barrier_wait(&barreiraThread);
}

long int* PthPrefixSum (long int* vEntrada, int nTotalElements, int nThreads){
	//cria um vetor maximo e um vetor resultante
	long int* vSaida	= malloc (nTotalElements*sizeof(long int));
	long int* vMaximos	= malloc (nThreads*sizeof(long int));

	//nThreads + 1 = threads + main
	pthread_barrier_init(&barreiraThread, NULL, nThreads+1);

	//laço que cria as threads que fazem a soma parcial e retornam o máximo daquela area em um vetor de maximos
	for (int i = 0 ; i < nThreads ; i++){
		long int* argumentos = malloc (5 * sizeof(long int));
		argumentos[0] = i * (nTotalElements / nThreads);				//comeco
		argumentos[1] = argumentos[0] + (nTotalElements / nThreads);	//fim
		argumentos[2] = (long int) vEntrada;
		argumentos[3] = (long int) vSaida;
		argumentos[4] = (long int) &(vMaximos[i]);

		pthread_create (&vThread[i], NULL, PrefixSumPart, (void *) argumentos);
	}

	//espera as threads fazendo a soma...
	pthread_barrier_wait(&barreiraThread);
	pthread_barrier_destroy(&barreiraThread);

	//calcula a soma de prefixos dos maximos
	sumMax (vMaximos, nThreads);

	#ifdef DEBUGPRINT

	fprintf(stdout, "\nOutput antes da soma dos max:\n");
	imprimeVetor (vSaida, nTotalElements);

	#endif

	//nThreads + 1 = threads + main
	pthread_barrier_init(&barreiraThread, NULL, nThreads+1);
	//soma paralelamente as somas de prefixos de maximos nas partes
	for (int i = 0 ; i < nThreads ; i++){
		long int* argumentos = malloc (5 * sizeof(long int));
		argumentos[0] = i * (nTotalElements / nThreads);				//comeco
		argumentos[1] = argumentos[0] + (nTotalElements / nThreads);	//fim
		argumentos[2] = i;
		argumentos[3] = (long int) vSaida;
		argumentos[4] = (long int) vMaximos;

		pthread_create (&vThread[i+nThreads], NULL, sumMaxPrefix, (void *) argumentos);
	}

	pthread_barrier_wait(&barreiraThread);
	pthread_barrier_destroy(&barreiraThread);

	#ifdef DEBUGPRINT

	fprintf(stdout, "\nvMaximos:\n");
	imprimeVetor (vMaximos, nThreads);

	#endif

	//retorna o vetor resultante
	return vSaida;
}

int verifica_corretude(long int* OutputVector, int nTotalElements){
	for (int i = 1 ; i < nTotalElements ; i++)
		if (OutputVector[i] != OutputVector[i-1] + i + 1)
			return 0;

	return 1;
}

int main (int argc, char *argv[]){
	//o programa deve ser chamado com 2 argumentos: nTotalElements nThreads
	if (argc != 3){
		fprintf(stderr, "A chamada nao possui o numero correto de argumentos :(\n");
		exit(1);
	}

	int nTotalElements	= atoi(argv[1]);
	int nThreads		= atoi(argv[2]);

	//correcao de erros
	fprintf (stdout, "Rodando: vetor de %d elementos com %d threads\n", nTotalElements, nThreads);

	long int *InputVector = malloc (nTotalElements*sizeof(long int));

	//gera o vetor
	InputVector[0] = 1;
	for( int i=0; i<nTotalElements ; i++ )
		InputVector[i] = i+1;

	chronometer_t chronoThreads;
	chrono_reset (&chronoThreads);

	chrono_start (&chronoThreads);

	//vetor resultante
	long int *OutputVector = PthPrefixSum (InputVector, nTotalElements, nThreads);

	chrono_stop (&chronoThreads);

	double tempo 	= (double) chrono_gettotal (&chronoThreads);
	double op		= tempo / nTotalElements;

	fprintf(stdout, "Tempo total:	%lf nanosecs\n", tempo);
	fprintf(stdout, "OPS:		%lf OP/nanosecs\n", op);
	
	#ifdef DEBUGPRINT

	fprintf(stdout, "\nInput:\n");
	imprimeVetor (InputVector, nTotalElements);
	fprintf(stdout, "\nOutput:\n");
	imprimeVetor (OutputVector, nTotalElements);

	#endif

	if (verifica_corretude(OutputVector, nTotalElements))
		fprintf (stdout, "ta certo lol\n");
	else
		fprintf (stdout, "rapaz...\n");

	return 0;
}
