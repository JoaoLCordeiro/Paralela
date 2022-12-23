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

void TrataEntradas (int argc, char* argv[], int* nmsg, int* tmsg, int* npro, int* bloq){
	if ((argc != 5) && (argc != 4)){
		fprintf(stderr, "A chamada nao possui o numero correto de argumentos :(\n");
		exit(1);
	}

	*nmsg = atoi(argv[1]);
	if (*nmsg % 2 != 0){
		fprintf(stderr, "Número de mensagens inválido (%d não é múltiplo de 2)\n", *nmsg);
		exit(1);
	}

	*tmsg = atoi(argv[2]);
	if (*tmsg % 8 != 0){
		fprintf(stderr, "Tamanho das mensagens inválido (%d não é múltiplo de 8)\n", *tmsg);
		exit(1);
	}

	*npro = atoi(argv[3]);
	if (*npro != 2){
		fprintf(stderr, "Número de processos MPI inválido (%d é diferente de 2)\n", *npro);
		exit(1);
	}

	*bloq = 1;	//DEFAULT

	if (argc == 5){
		if (strcmp(argv[4], "-nbl") == 0)
			*bloq = 0;
	}

	//fprintf(stderr, "nmsg = %d\ntmsg = %d\nnpro = %d\nbloq = %d\n", *nmsg, *tmsg, *npro, *bloq);
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
	int nmsg, tmsg, npro, bloq;
	TrataEntradas (argc, argv, &nmsg, &tmsg, &npro, &bloq);
	//passando daqui, temos certeza que as entradas são válidas

	//inicializa e guarda quantos processos existem e o rank do processo atual
	MPI_Init( &argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &nProc);
	MPI_Comm_rank (MPI_COMM_WORLD, &rankProc);

	//fprintf (stderr, "Passou a parte que pega o rank e o size\n");

	int ni	 = tmsg/sizeof(TYPEMSG);
	int otherProc = (rankProc + 1) % 2;
	TYPEMSG* buffMsgSend = CriaMsg(ni);
	TYPEMSG* buffMsgRecv = (TYPEMSG*) calloc (ni, sizeof(TYPEMSG));
	MPI_Status statusRecv;

	if (rankProc == 0)
		verificaVetores( buffMsgSend, buffMsgRecv, ni);
	else //(rankProc == 1)
		verificaVetores( buffMsgRecv, buffMsgSend, ni);

	//fprintf (stderr, "Passou o cria msg e o malloc\n");

	if (bloq == 0){
		MPI_Barrier(MPI_COMM_WORLD);
	}

	if (rankProc == 0){
		chrono_reset(&cronometro);
		chrono_start(&cronometro);
	}

	//é bloqueante ou não
	if (bloq == 1)
		if (rankProc == 0)
			for (int i_msg = 0 ; i_msg < nmsg ; i_msg++){
				MPI_Ssend (buffMsgSend, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD);
				MPI_Recv (buffMsgRecv, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD, &statusRecv);
			}
		else
			for (int i_msg = 0 ; i_msg < nmsg ; i_msg++){
				MPI_Recv (buffMsgRecv, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD, &statusRecv);
				MPI_Ssend (buffMsgSend, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD);
			}
	else
		for (int i_msg = 0 ; i_msg < nmsg ; i_msg++){
			MPI_Bsend (buffMsgSend, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD);
			MPI_Recv (buffMsgRecv, ni, MPI_LONG, otherProc, 0, MPI_COMM_WORLD, &statusRecv);
		}

	if (bloq == 0){
		MPI_Barrier(MPI_COMM_WORLD);
	}

	if (rankProc == 0){
		chrono_stop(&cronometro);
		double tempo = (double) chrono_gettotal(&cronometro) / (1000 * 1000 * 1000);
		double vazao = nmsg / tempo;

		fprintf (stdout, "Tempo e vazao:	%f	%f\n", nmsg, tempo, vazao);
	}

	if (rankProc == 0)
		verificaVetores( buffMsgSend, buffMsgRecv, ni);
	else //(rankProc == 1)
		verificaVetores( buffMsgRecv, buffMsgSend, ni);

	//fprintf (stdout, "Processo %d:	vetorSend[0] = %ld	vetorRecv[0] = %ld\n", rankProc, buffMsgSend[0], buffMsgRecv[0]);

	//fprintf (stderr, "Passou a troca de mensagens\n");

	MPI_Finalize();

	return 0;
}
