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

int main (int argc, char* argv[]){
	int nmsg, tmsg, raiz;
	TrataEntradas (argc, argv, &nmsg, &tmsg, &raiz);
	//passando daqui, temos certeza que as entradas são válidas

	//inicializa e guarda quantos processos existem e o rank do processo atual
	MPI_Init( &argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &nProc);
	MPI_Comm_rank (MPI_COMM_WORLD, &rankProc);

	int ni	 = tmsg/sizeof(TYPEMSG);

	TYPEMSG* buffMsgSend = CriaMsg(ni);
	TYPEMSG* buffMsgRecv = (TYPEMSG*) malloc (ni * sizeof(TYPEMSG));

	MPI_Barrier(MPI_COMM_WORLD);

	if (rankProc == 0){
		chrono_reset(&cronometro);
		chrono_start(&cronometro);
	}

	MPI_Status statusRecv;

	if (rankProc == 0)
		for (int imsg = 0 ; imsg < nmsg ; imsg++){	
			MPI_Ssend (buffMsgSend, ni, MPI_LONG, 1, 0, MPI_COMM_WORLD);
			MPI_Recv (buffMsgRecv, ni, MPI_LONG, 1, 0, MPI_COMM_WORLD, &statusRecv);
		}
	else	//rankProc == 1
		for (int imsg = 0 ; imsg < nmsg ; imsg++){	
			MPI_Recv (buffMsgRecv, ni, MPI_LONG, 0, 0, MPI_COMM_WORLD, &statusRecv);
			MPI_Ssend (buffMsgSend, ni, MPI_LONG, 0, 0, MPI_COMM_WORLD);
		}

	MPI_Barrier(MPI_COMM_WORLD);

	if (rankProc == 0){
		chrono_stop(&cronometro);
		double tempoMS	= (double) chrono_gettotal(&cronometro) / (1000*1000);
		//double tempoMS	= tempoS * 1000;
		double vazao	= ((tmsg*nmsg)/tempoMS)*(nProc-1);

		fprintf (stdout, "\nNP:	%d	RAIZ:	%d\n", nProc, raiz);
		fprintf (stdout, "Tempo:	%f	Vazao:	%f\n", tempoMS, vazao);
	}

	MPI_Finalize();

	return 0;
}
