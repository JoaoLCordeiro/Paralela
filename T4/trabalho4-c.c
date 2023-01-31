#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "chrono.c"

#define TYPEMSG long int

int nProc, rankProc;

chronometer_t cronometro;

void verificaVetores( long ping[], long pong[], int ni ){
   static int twice = 0;
   int ping_ok = 1;
   int pong_ok = 1;
   int i, rank;
      
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );   
   
   if( twice == 0 ) {
  
      if (rank == 0) {
      
          for( i=0; i<ni; i++ ) {
            if( ping[i] != i+1 ) { ping_ok = 0; break; }
            if( pong[i] != 0   ) { pong_ok = 0; break; }
          }
          if( !ping_ok )
             fprintf(stderr, 
               "--------- rank 0, initial value of ping[%d] = %ld (wrong!)\n", i, ping[i] );
          if( !pong_ok )
             fprintf(stderr, 
               "--------- rank 0, initial value of pong[%d] = %ld (wrong!)\n", i, pong[i] );
          if( ping_ok && pong_ok )
             fprintf(stderr, 
               "--------- rank 0, initial value of ping and pong are OK\n" );

      } else if (rank == 1) {
      
          for( i=0; i<ni; i++ ) {
            if( ping[i] != 0      ) { ping_ok = 0; break; }
            if( pong[i] != i+ni+1 ) { pong_ok = 0; break; }
          }
          if( !ping_ok )
             fprintf(stderr, 
               "--------- rank 1, initial value of ping[%d] = %ld (wrong!)\n", i, ping[i] );
          if( !pong_ok )
             fprintf(stderr, 
               "--------- rank 1, initial value of pong[%d] = %ld (wrong!)\n", i, pong[i] );
          if( ping_ok && pong_ok )
             fprintf(stderr, 
               "--------- rank 1, initial values of ping and pong are OK\n" );
      }          
   }   // end twice == 0
   
   if( twice == 1 ) {
  
          for( i=0; i<ni; i++ ) {
            if( ping[i] != i+1      ) { ping_ok = 0; break; }
            if( pong[i] != i+ni+1   ) { pong_ok = 0; break; }
          }
          if( !ping_ok )
             fprintf(stderr, 
               "--------- rank %d, FINAL value of ping[%d] = %ld (wrong!)\n", rank, i, ping[i] );
          if( !pong_ok )
             fprintf(stderr, 
               "--------- rank %d, FINAL value of pong[%d] = %ld (wrong!)\n", rank, i, pong[i] );
          if( ping_ok && pong_ok )
             fprintf(stderr, 
               "--------- rank %d, FINAL values of ping and pong are OK\n", rank );

   }  // end twice == 1
   
   ++twice;
   if( twice > 2 )
      fprintf(stderr, 
               "--------- rank %d, verificaVetores CALLED more than 2 times!!!\n", rank );     
}  

void TrataEntradas (int argc, char* argv[], int* nmsg, int* tmsg, int* raiz){
	//temos 3 argumentos opicionais, onde -r tem q ter um numero dps
	if ((argc != 5) && (argc != 3)){
		fprintf(stderr, "A chamada nao possui o numero correto de argumentos :(\n");
		exit(1);
	}

	*nmsg = atoi(argv[1]);

	*tmsg = atoi(argv[2]);
	if (*tmsg % 8 != 0){
		fprintf(stderr, "Tamanho das mensagens inválido (%d não é múltiplo de 8)\n", *tmsg);
		exit(1);
	}

	*raiz = 0;
	if (argc == 5){
		if (strcmp(argv[3], "-r") == 0)
			*raiz = atoi(argv[4]);
		else
			fprintf(stderr, "Terceiro argumento errado: %s (esperado '-r')\n", argv[3]);
	}

	//fprintf(stderr, "nmsg = %d\ntmsg = %d\nraiz = %d\n", *nmsg, *tmsg, *raiz);
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

//retorna o teto do logaritmo base 2, usado para descobrir a fase 
//do processo e pra descobrir o numero de fases necessárias
int tetoLog (int n){
	double nd = n;
	int res = 0;

	while (nd > 1){
		res++;
		nd = nd/2;
	}

	return res;
}

//funcao que descobre a fase em que o processo atual vai receber a mensagem
int descobreFase (int rankProc, int raiz, int nProc){
	int distancia;
	if (rankProc >= raiz)
		distancia = rankProc - raiz;
	else
		distancia = rankProc + nProc - raiz;
	
	int res;
	if (distancia == 0)
		res = distancia;
	else
	 	res = tetoLog(distancia+1);

	return res;
}

//funcao que retorna 2^n
int pow2 (int n){
	if (n == 0)
		return 1;
	else{
		int res = 1;
		for (int i = 0 ; i < n ; i++)
			res = res * 2;

		return res;
	}
}

//função de debug
void calculaNumeros (int nProc, int raiz, int numFases){
	int origemMsg, destinoMsg;

	for (int i = 0 ; i < nProc ; i++){

		int faseComeco = descobreFase(i, raiz, nProc);
		fprintf (stdout, "\n\nProcesso:	%d\nComeco:	%d\n", i, faseComeco);

		if (i != raiz){
			origemMsg = ( i - pow2(faseComeco-1) ) % nProc;
			if (origemMsg < 0)
				origemMsg = nProc + origemMsg;
			fprintf (stdout, "Recebe do:	%d\n", origemMsg);
		}

		//envia as mensagens que tem que mandar
		for (; faseComeco < numFases ; faseComeco++){
			destinoMsg = (pow2(faseComeco) + i) % nProc;
			if (faseComeco == numFases - 1){
				int distancia;
				if (raiz <= i)
					distancia = i - raiz;
				else
					distancia = i + nProc - raiz;
				if (distancia + pow2(faseComeco) < nProc)
					fprintf (stdout, "Manda pro:	%d\n", destinoMsg);
			}
			else
				fprintf (stdout, "Manda pro:	%d\n", destinoMsg);
		}
	}
}

int main (int argc, char* argv[]){
	int nmsg, tmsg, raiz;
	TrataEntradas (argc, argv, &nmsg, &tmsg, &raiz);
	//passando daqui, temos certeza que as entradas são válidas

	//inicializa e guarda quantos processos existem e o rank do processo atual
	MPI_Init( &argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &nProc);
	MPI_Comm_rank (MPI_COMM_WORLD, &rankProc);

	int ni	 = tmsg/sizeof(TYPEMSG);

	TYPEMSG* buffMsg;
	MPI_Status statusRecv;

	if (rankProc == raiz)
		buffMsg = CriaMsg(ni);
	else
		buffMsg = (TYPEMSG*) calloc (ni, sizeof(TYPEMSG));

	MPI_Barrier(MPI_COMM_WORLD);

	if (rankProc == 0){
		chrono_reset(&cronometro);
		chrono_start(&cronometro);
	}

	//aqui descobrimos quantas fases de envio terao
	int numFases = tetoLog(nProc);
	
	//função para debug
	//if (rankProc == 0)
	//	calculaNumeros (nProc, raiz, numFases);

	int faseComeco;
	int destinoMsg;
	int origemMsg;

	for (int imsg = 0 ; imsg < nmsg ; imsg++){
		//começamos o broadcast

		//aqui descobrimos em qual fase o processo atual começa a ouvir
		faseComeco = descobreFase(rankProc, raiz, nProc);

		//se o processo nao for a raiz, começa ouvindo
		if (rankProc != raiz){
			origemMsg = ( rankProc - pow2(faseComeco-1) ) % nProc;

			//caso a origem seja maior q o destino, o calculo dará negativo
			if (origemMsg < 0)
				origemMsg = nProc + origemMsg;
			MPI_Recv (buffMsg, ni, MPI_LONG, origemMsg, 0, MPI_COMM_WORLD, &statusRecv);
		}

		//envia as mensagens que tem que mandar
		for (; faseComeco < numFases ; faseComeco++){
			destinoMsg = (pow2(faseComeco) + rankProc) % nProc;

			//se for a ultima fase de transmissoes, faz umas verificacoes
			if (faseComeco == numFases - 1){
				int distancia;
				if (raiz <= rankProc)
					distancia = rankProc - raiz;
				else
					distancia = rankProc + nProc - raiz;

				//calcula se a mensagem deve ser realmente enviada
				//(casos onde nProc nao eh potencia de 2)
				if (distancia + pow2(faseComeco) < nProc)
					MPI_Ssend (buffMsg, ni, MPI_LONG, destinoMsg, 0, MPI_COMM_WORLD);
			}
			else
				MPI_Ssend (buffMsg, ni, MPI_LONG, destinoMsg, 0, MPI_COMM_WORLD);
		}

		//terminamos o broadcast

		MPI_Barrier(MPI_COMM_WORLD);

	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (rankProc == 0){
		chrono_stop(&cronometro);
		double tempoS	= (double) chrono_gettotal(&cronometro) / (1000 * 1000 * 1000);
		double tempoMS	= tempoS * 1000;
		double vazao	= ((tmsg*nmsg)/tempoMS)*(nProc-1);

		fprintf (stdout, "\nNP:	%d	RAIZ:	%d\n", nProc, raiz);
		fprintf (stdout, "Tempo:	%f	Vazao:	%f\n", tempoS, vazao);
	}

	MPI_Finalize();

	return 0;
}
