#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "chrono.c"

//#define DEBUGPRINT
#define MAX_THREADS 64

pthread_t vThread[MAX_THREADS];
pthread_barrier_t barreiraThread;

void imprimeVetor (long int *vetor, long int n){
	fprintf(stdout, "Vetor:	");
	for (int i = 0 ; i < n ; i++)
		fprintf(stdout, " %ld", vetor[i]);
	fprintf(stdout, "\n");
}

void sumMax (long int* vMax, long int elementos){
	for (int i = 1 ; i < elementos ; i++)
		vMax[i] += vMax[i-1];
}

void PrefixSumPart (long int* argum){

	//coloca os argumentos em variáveis
	long int	comeco	= argum[0];
	long int	fim		= argum[1];
	long int*	vetorE	= (long int*) argum[2];
	long int*	vetorS	= (long int*) argum[3];

	//faz a soma de prefixos na area desejada
	vetorS[comeco] = vetorE[comeco];
	for (int i = comeco+1 ; i < fim ; i++){
		vetorS[i] = vetorE[i] + vetorS[i-1];
	}

	//coloca o ultimo valor nessa posicao do argum
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

	//a soma nao acontece no primeiro intervalo
	if (thread != 0)
		for (int posicao = comeco ; posicao < fim ; posicao++)
			vetorS[posicao] += vMax[thread-1];

	free(argum);
	
	pthread_barrier_wait(&barreiraThread);
}

long int* PthPrefixSum (long int* vEntrada, long int nTotalElements, long int nThreads){
	//cria um vetor maximo e um vetor saida
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

	//retorna o vetor resultante
	return vSaida;
}

int verifica_corretude(long int* OutputVector, long int nTotalElements){
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

	long int nTotalElements	= atoi(argv[1]);
	long int nThreads		= atoi(argv[2]);

	//correcao de erros
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

	double tempo 	= (double) chrono_gettotal (&chronoThreads) / (1000 * 1000 * 1000);
	double op		= nTotalElements / tempo;

	//fprintf (stdout, "Rodando: vetor de %ld elementos com %ld threads\n", nTotalElements, nThreads);
	//fprintf(stdout, "Tempo total:	%lf 	nanosecs\n", tempo);
	//fprintf(stdout, "OPS:			%lf 		OP/nanosecs\n", op);

	//print para gerar os dados da planilha
	fprintf(stdout, "%lf,%lf\n", tempo, op);

	// codigo de verificação
    // (verifica o vetor de saída se contém uma soma de prefixos 
    //   correspondente ao vetor de entrada)
    long int last = 0;
    for( long int i=0; i<nTotalElements ; i++ ) {
           if( OutputVector[i] != (InputVector[i] + last) ) {
               fprintf( stderr, "Out[%ld]= %ld (wrong result!) %ld\n", i, OutputVector[i] , InputVector[i] + last);
               break;
           }
           last = OutputVector[i];    
    } 

	return 0;
}
