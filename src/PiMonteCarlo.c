/*
 * PiMonteCarlo.c
 * Classe Main che calcola il metodo di Monte Carlo, contenente sia la versione per il calcolo sequenziale che quello parallelo
 * Author: Scavone Francesca, 0522500705
*/

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <time.h>
#include "functionsMC.h"

int main(int argc, char** argv){
	
	long int n_iter = 0; 				//iterazioni totali
	int rank;					//rank dei processori
	int p;						//processori
	long double pi;					//pi greco
	
	int *buffer;					//per la divisione nei processori
	int count_red;					//variabile di ricezione dei punti calcolati da ogni processore
	int iteration;					//iterazioni di ogni processore
	
	double start_time = 0.0, end_time = 0.0;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if(rank == 0){

		if(argc != 2){
			printf("*** ERROR *** \nla forma dovrebbe essere: %s <numero iterazioni totali>\n", argv[0]);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_COUNT);
		}

		//Il numero di iterazioni viene prelevato dalla riga di comando
		n_iter = atol(argv[1]);	
		
		if(p == 1){
			//Esecuzione del codice sequenziale
			sequenzialMC(start_time, end_time, pi, n_iter);						
			MPI_Finalize();
			return 0;
		}

		printf("-------------------------------------\n");
		printf("PARALLELO START\n");
		printf("-------------------------------------\n");

		buffer = (int *) malloc (p * sizeof(int));

		start_time = MPI_Wtime();

		//Calcolo della parte da assegnare ad ogni processore
		partCalculator(buffer, p, n_iter);										
	}

	MPI_Scatter(buffer, 1, MPI_INT, &iteration, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Calcolo del metodo di Monte Carlo da parte di ogni processore
	long int res = monteCarloMethod(iteration);
	
	//Il processore 0 effettua la somma di tutti i risultati ricevuti dagli altri processori
	MPI_Reduce(&res, &count_red, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);		
	
	if(rank == 0){
		//Il processore 0 calcola il valore di PI
		pi = ((long double) count_red / (long double) n_iter) * 4.0;
		
		end_time = MPI_Wtime();
		double exe_time = end_time - start_time;
	   	
	   	printf("PI:             %Lf\nIterazioni: 	%li\nTempo: 		%.6f\n", pi, n_iter, exe_time);
	   	
	   	free(buffer);

	   	printf("-------------------------------------\n");
		printf("PARALLELO END\n");
		printf("-------------------------------------\n");
   }
	
	MPI_Finalize();
	return 0;
}

