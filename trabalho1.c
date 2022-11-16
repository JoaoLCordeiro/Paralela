#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
	long int	max		= vetorE[comeco];

	//faz a soma de prefixos na area desejada

	vetorS[comeco] = vetorE[comeco];
	for (int i = comeco+1 ; i < fim ; i++){
		vetorS[i] = vetorE[i] + vetorS[i-1];

		if (vetorS[i] > max)
			max = vetorS[i];
	}

	//coloca o maximo nessa posicao
	*((long int *) argum[4]) = max;

	//no fim, libera o vetor de argumentos
	free (argum);

	pthread_barrier_wait(&barreiraThread);
	return;
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

	fprintf(stdout, "Terminou a thread das somas com comeco em %ld\n", comeco);
	pthread_barrier_wait(&barreiraThread);
}

long int* PthPrefixSum (long int* vEntrada, int nTotalElements, int nThreads){
	//cria um vetor maximo e um vetor resultante
	long int* vSaida	= malloc (nTotalElements*sizeof(long int));
	long int* vMaximos	= malloc (nThreads*sizeof(long int));

	pthread_barrier_init(&barreiraThread, NULL, nThreads);

	//laço que cria as threads que fazem a soma parcial e retornam o máximo daquela area em um vetor de maximos
	for (int i = 0 ; i < nThreads ; i++){
		long int* argumentos = malloc (4 * sizeof(long int));
		argumentos[0] = i * (nTotalElements / nThreads);				//comeco
		argumentos[1] = argumentos[0] + (nTotalElements / nThreads);	//fim
		argumentos[2] = (long int) vEntrada;
		argumentos[3] = (long int) vSaida;
		argumentos[4] = (long int) &(vMaximos[i]);

		fprintf (stdout, "Criando a função com comeco em %ld\n", argumentos[0]);
		pthread_create (&vThread[i], NULL, PrefixSumPart, (void *) argumentos);
		fprintf (stdout, "Criou a função com comeco em %ld\n", argumentos[0]);
	}

	//espera as threads fazendo a soma...
	fprintf(stdout, "Esperando as %d threads...\n", nThreads);
	pthread_barrier_wait(&barreiraThread);
	pthread_barrier_destroy(&barreiraThread);
	fprintf(stdout, "Esperou e destruiu :)\n");

	//calcula a soma de prefixos dos maximos
	sumMax (vMaximos, nThreads);

	fprintf(stdout, "\nOutput antes da soma dos max:\n");
	imprimeVetor (vSaida, nTotalElements);

	pthread_barrier_init(&barreiraThread, NULL, nThreads);
	//soma paralelamente as somas de prefixos de maximos nas partes
	for (int i = 0 ; i < nThreads ; i++){
		long int* argumentos = malloc (4 * sizeof(long int));
		argumentos[0] = i * (nTotalElements / nThreads);				//comeco
		argumentos[1] = argumentos[0] + (nTotalElements / nThreads);	//fim
		argumentos[2] = i;
		argumentos[3] = (long int) vSaida;
		argumentos[4] = (long int) vMaximos;

		fprintf (stdout, "Criando a função com comeco em %ld\n", argumentos[0]);
		pthread_create (&vThread[i+nThreads], NULL, sumMaxPrefix, (void *) argumentos);
		fprintf (stdout, "Criou a função com comeco em %ld\n", argumentos[0]);
	}

	fprintf(stdout, "Esperando as %d threads...\n", nThreads);
	pthread_barrier_wait(&barreiraThread);
	pthread_barrier_destroy(&barreiraThread);
	fprintf(stdout, "Esperou e destruiu :)\n");

	fprintf(stdout, "\nvMaximos:\n");
	imprimeVetor (vMaximos, nTotalElements);

	//retorna o vetor resultante
	return vSaida;
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
	fprintf (stdout, "Argumentos: %d	%d\n", nTotalElements, nThreads);
	fprintf (stdout, "Tamanhos: %ld	%ld\n", sizeof(long int), sizeof(long int *));

	long int *InputVector = malloc (nTotalElements*sizeof(long int));

	//gera o vetor	
	for( int i=0; i<nTotalElements ; i++ )
		InputVector[i] = i+1;

	//vetor resultante
	long int *OutputVector = PthPrefixSum (InputVector, nTotalElements, nThreads);

	fprintf(stdout, "\nInput:\n");
	imprimeVetor (InputVector, nTotalElements);
	fprintf(stdout, "\nOutput:\n");
	imprimeVetor (OutputVector, nTotalElements);
	return 0;
}