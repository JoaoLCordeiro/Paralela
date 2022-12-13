// TRABALHO2: CI316 1o semestre 2022
// Aluno: Joao Lucas Cordeiro
// GRR: 20190427
//

	///////////////////////////////////////
	///// ATENCAO: NAO MUDAR O MAIN   /////
    ///////////////////////////////////////

#include <iostream>           // WILL USE SOME C++ features
#include <typeinfo>
//        template <typename T> std::string type_name();

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>



#include "chrono.c"

//#define DEBUG 2
#define DEBUG 0

#define MAX_THREADS 64

#define MAX_TOTAL_ELEMENTS (500 * 1000 * 1000) // para maquina de 16GB de RAM 
//#define MAX_TOTAL_ELEMENTS (200 * 1000 * 1000) // para maquina de 8GB de RAM 
											   

// ESCOLHA o tipo dos elementos usando o #define TYPE adequado abaixo
//    a fazer a SOMA DE PREFIXOS:											   
#define TYPE long                        // OBS: o enunciado pedia ESSE (long)
//#define TYPE long long
//#define TYPE double
// OBS: para o algoritmo da soma de prefixos
//  os tipos abaixo podem ser usados para medir tempo APENAS como referência 
//  pois nao conseguem precisao adequada ou estouram capacidade 
//  para quantidades razoaveis de elementos
//#define TYPE float
// #define TYPE int



int nThreads;		// numero efetivo de threads
					// obtido da linha de comando
int nTotalElements; // numero total de elementos
					// obtido da linha de comando

//volatile 
TYPE *InputVector;	// will use malloc e free to allow large (>2GB) vectors
//volatile 
TYPE *OutputVector;	// will use malloc e free to allow large (>2GB) vectors


chronometer_t parallelPrefixSumTime;
pthread_barrier_t myBarrier;
volatile TYPE partialSum[MAX_THREADS];
pthread_t vThread[MAX_THREADS];
   
int min(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

void verifyPrefixSum( const TYPE *InputVec, 
                      const TYPE *OutputVec, 
                      long nTotalElmts )
{
    volatile TYPE last = InputVec[0];
    int ok = 1;
    for( long i=1; i<nTotalElmts ; i++ ) {
           if( OutputVec[i] != (InputVec[i] + last) ) {
               fprintf( stderr, "In[%ld]= %ld\n"
                                "Out[%ld]= %ld (wrong result!)\n"
                                "Out[%ld]= %ld (ok)\n"
                                "last=%ld\n" , 
                                     i, InputVec[i],
                                     i, OutputVec[i],
                                     i-1, OutputVec[i-1],
                                     last );
               ok = 0;
               break;
           }
           last = OutputVec[i];    
    }  
    if( ok )
       printf( "\nPrefix Sum verified correctly.\n" );
    else
       printf( "\nPrefix Sum DID NOT compute correctly!!!\n" );   
}

void* PartialSum (void* argum){
	//coloca os argumentos em variáveis
	long int* vetorArg = (long int*) argum;

	long int comeco	= vetorArg[0];
	long int fim	= vetorArg[1];
	long int thread	= vetorArg[2];

	fprintf (stdout, "PartialSum de %d até %d na thread %d\n", comeco, fim, thread);

	long int sum = 0;

	//calcula a soma parcial da thread
	for (int i = comeco ; i < fim ; i++){
		sum += InputVector[i];
	}

	//coloca a soma parcial no vetor
	partialSum[thread] = sum;

	//espera
	pthread_barrier_wait(&myBarrier);
	pthread_exit(NULL);
}

void* FinalSum (void* argum){
	//coloca os argumentos em variáveis
	long int* vetorArg = (long int*) argum;

	long int comeco	= vetorArg[0];
	long int fim	= vetorArg[1];
	long int thread	= vetorArg[2];

	fprintf (stdout, "FinalSum de %d até %d na thread %d\n", comeco, fim, thread);

	//pega a soma parcial da thread
	long int myPartialSum = 0;
	if (thread > 0)
		myPartialSum = partialSum[thread-1];

	fprintf (stdout, "Na thread %d a partial sum é %d\n", thread, myPartialSum);

	//faz a soma de prefixos
	OutputVector[comeco] = InputVector[comeco] + myPartialSum;
	for (int i = comeco+1 ; i < fim ; i++){
		OutputVector[i] = InputVector[i] + OutputVector[i-1];
	}

	//espera
	pthread_barrier_wait(&myBarrier);
	pthread_exit(NULL);
}

void ParallelPrefixSumPth( const TYPE *InputVec, 
                           const TYPE *OutputVec, 
                           long nTotalElmts )
{
   pthread_t Thread[MAX_THREADS];
   int my_thread_id[MAX_THREADS];

   long int nElements = nTotalElements;

   fprintf (stdout, "comecou o prefixsum\n");
   
   // inicializar a barreira
   pthread_barrier_init(&myBarrier, NULL, nThreads+1);	//+1 por causa da principal


	fprintf (stdout, "iniciou a primeira barreira\n");

   ///////////////// INCLUIR AQUI SEU CODIGO da V2 /////////

   // criar as threads aqui!
   //laço que cria as threads que fazem a soma parcial e retornam o máximo daquela area em um vetor de maximos
	for (int i = 0 ; i < nThreads ; i++){
		long int* argumentos = (long int *) malloc (3 * sizeof(long int));
		argumentos[0] = ((i * nElements) / nThreads);		//comeco
		argumentos[1] = (((i+1) * nElements) / nThreads);	//fim
		argumentos[2] = i;									//num da thread

		pthread_create (&vThread[i], NULL, PartialSum, (void *) argumentos);
	}

	fprintf (stdout, "esperando na main...\n");
	pthread_barrier_wait(&myBarrier);

	pthread_barrier_destroy(&myBarrier);

	fprintf (stdout, "destruiu a barreira\n");
	pthread_barrier_init(&myBarrier, NULL, nThreads+1);
	fprintf (stdout, "iniciou a barreira novamente\n");

	fprintf (stdout, "PartialSum: [%d	", partialSum[0]);
	//faz a soma de prefixos nas somas parciais
	for (int i = 1 ; i < nThreads ; i++){
		partialSum[i] += partialSum[i-1];
		fprintf (stdout, ",%d	", partialSum[i]);
	}
	fprintf (stdout, "]\n");

	fprintf (stdout, "faz a soma de prefixos no partialSum\n");

	for (int i = 0 ; i < nThreads ; i++){
		long int* argumentos = (long int *) malloc (3 * sizeof(long int));
		argumentos[0] = ((i * nElements) / nThreads);		//comeco
		argumentos[1] = (((i+1) * nElements) / nThreads);	//fim
		argumentos[2] = i;									//num da thread

		fprintf (stdout, "vai criar a thread %d\n", i);
		pthread_create (&vThread[i], NULL, FinalSum, (void *) argumentos);
	}

	fprintf (stdout, "esperando de novo na main...\n");
	pthread_barrier_wait(&myBarrier);

   // voce pode criar outras funcoes para as suas threads
   
   // fazer join das threads aqui!
   //////////////////////////////////////////////////////////
   
    pthread_barrier_destroy(&myBarrier);

}

int main(int argc, char *argv[])
{
	long i;
	
	///////////////////////////////////////
	///// ATENCAO: NAO MUDAR O MAIN   /////
    ///////////////////////////////////////

	if (argc != 3)
	{
		printf("usage: %s <nTotalElements> <nThreads>\n",
			   argv[0]);
		return 0;
	}
	else
	{
		nThreads = atoi(argv[2]);
		if (nThreads == 0)
		{
			printf("usage: %s <nTotalElements> <nThreads>\n",
				   argv[0]);
			printf("<nThreads> can't be 0\n");
			return 0;
		}
		if (nThreads > MAX_THREADS)
		{
			printf("usage: %s <nTotalElements> <nThreads>\n",
				   argv[0]);
			printf("<nThreads> must be less than %d\n", MAX_THREADS);
			return 0;
		}
		nTotalElements = atoi(argv[1]);
		if (nTotalElements > MAX_TOTAL_ELEMENTS)
		{
			printf("usage: %s <nTotalElements> <nThreads>\n",
				   argv[0]);
			printf("<nTotalElements> must be up to %d\n", MAX_TOTAL_ELEMENTS);
			return 0;
		}
	}

        // allocate InputVector AND OutputVector
        InputVector = (TYPE *) malloc( nTotalElements*sizeof(TYPE) );
        if( InputVector == NULL )
            printf("Error allocating InputVector of %d elements (size=%ld Bytes)\n",
                     nTotalElements, nTotalElements*sizeof(TYPE) );
        OutputVector = (TYPE *) malloc( nTotalElements*sizeof(TYPE) );
        if( OutputVector == NULL )
            printf("Error allocating OutputVector of %d elements (size=%ld Bytes)\n",
                     nTotalElements, nTotalElements*sizeof(TYPE) );


//    #if DEBUG >= 2 
        // Print INFOS about the reduction
        TYPE myType;
        long l;
        std::cout << "Using PREFIX SUM of TYPE ";
        
        if( typeid(myType) == typeid(int) ) 
		std::cout << "int" ;
	else if( typeid(myType) == typeid(long) ) 
		std::cout << "long" ;
	else if( typeid(myType) == typeid(float) ) 
		std::cout << "float" ;
	else if( typeid(myType) == typeid(double) ) 
		std::cout << "double" ;
	else if( typeid(myType) == typeid(long long) ) 
		std::cout << "long long" ;
	else 
	    std::cout << " ?? (UNKNOWN TYPE)" ;
	
	std::cout << " elements (" << sizeof(TYPE) 
	          << " bytes each)\n" << std::endl;
                  

	/*printf("reading inputs...\n");
	for (int i = 0; i < nTotalElements; i++)
	{
		scanf("%d", &InputVector[i]);
	}*/
	
	// initialize InputVector
	//srand(time(NULL));   // Initialization, should only be called once.
        
        int r;
	for (long i = 0; i < nTotalElements; i++){
	        r = rand();  // Returns a pseudo-random integer
	                     //    between 0 and RAND_MAX.
		InputVector[i] = (r % 1000) - 500;
		//InputVector[i] = 1; // i + 1;
	}

	printf("\n\nwill use %d threads to calculate prefix-sum of %d total elements\n", 
	                     nThreads, nTotalElements);

	chrono_reset(&parallelPrefixSumTime);
	chrono_start(&parallelPrefixSumTime);

            ParallelPrefixSumPth( InputVector, OutputVector, nTotalElements );

	// Measuring time of the parallel algorithm 
	//    including threads creation and joins...
	chrono_stop(&parallelPrefixSumTime);
	chrono_reportTime(&parallelPrefixSumTime, "parallelPrefixSumTime");

	// calcular e imprimir a VAZAO (numero de operacoes/s)
	double total_time_in_seconds = (double)chrono_gettotal(&parallelPrefixSumTime) /
								   ((double)1000 * 1000 * 1000);
	printf("total_time_in_seconds: %lf s\n", total_time_in_seconds);

	double OPS = (nTotalElements) / total_time_in_seconds;
	printf("Throughput: %lf OP/s\n", OPS);

	////////////
        verifyPrefixSum( InputVector, OutputVector, nTotalElements );
        //////////
    
    //#if NEVER
    #if DEBUG >= 2 
        // Print InputVector
        printf( "In: " );
	for (int i = 0; i < nTotalElements; i++){
		printf( "%d ", InputVector[i] );

	}
	printf( "\n" );

    	// Print OutputVector
    	printf( "Out: " );
	for (int i = 0; i < nTotalElements; i++){
		printf( "%d ", OutputVector[i] );
	}
	printf( "\n" );
    #endif
	return 0;
}
