#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 64

pthread_t vThread[MAX_THREADS];
pthread_barrier_t barreiraThread;

void PrefixSumPart (void* args){
	int		comeco	= (int)  args[0];
	int		fim		= (int)  args[1];
	int*	vetor	= (int*) args[2];
	int		max		= vetor[comeco];

	for (i = comeco+1 , i < fim , i++){
		vetor[i] += vetor[i-1];

		if (vetor[i] > max)
			max = vetor[i];
	}

	*(args[3]) = max;
}

int* PthPrefixSum (int* vEntrada, int nTotalElements, int nThreads){
	//cria um vetor maximo e um vetor resultante
	int* vSaida		= malloc (nTotalElements*sizeof(int));
	int* vMaximos	= malloc (nThreads*sizeof(int));

	//laço que cria as threads que fazem a soma parcial e retornam o máximo daquela area em um vetor de maximos
	for (int i = 0 ; i < nTotalElements ; i++){
		void* argumentos = malloc (3 * sizeof(int));
		argumentos[0] = i * (nTotalElements / nThreads);				//comeco
		argumentos[1] = argumentos[0] + (nTotalElements / nThreads);	//fim
		argumentos[2] = vEntrada;
		argumentos[3] = &(vMaximos[i]);

		pthread_create (vThread[i], NULL, PrefixSumPart, argumentos);
	}

	//espera as threads fazendo a soma...
	

	//calcula a soma de prefixos dos maximos

	//soma paralelamente as somas de prefixos de maximos nas partes

	//retorna o vetor resultante
}

int main (int argc, char *argv[]){
	//o programa deve ser chamado com 2 argumentos: nTotalElements nThreads
	if (argc != 3){
		fprintf(stderr, "A chamada nao possui o numero correto de argumentos :(\n");
	}

	int nTotalElements	= atoi(argv[1]);
	int nThreads		= atoi(argv[2]);

	//correcao de erros
	fprintf (stdout, "Argumentos: %d	%d\n", nTotalElements, nThreads);

	int *InputVector = malloc (nTotalElements*sizeof(int));

	//gera o vetor	
	for( int i=0; i<nTotalElements ; i++ )
		InputVector[i] = i+1;

	//vetor resultante
	int *OutputVector = PthPrefixSum (InputVector, nTotalElements, nThreads);

	return 0;
}
