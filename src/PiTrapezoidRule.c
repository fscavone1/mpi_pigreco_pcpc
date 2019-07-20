/*
 * PiTrapezoidRule.c
 * Classe Main che calcola la regola del Trapezoide, contenente sia la versione per il calcolo sequenziale che quello parallelo
 * Author: Scavone Francesca, 0522500705
*/

#include <stdio.h>
#include <math.h>
#include "mpi.h"
#include <stdlib.h>
#include "functionsTrap.h"

#define N 1E7
#define d 1E-7
#define d2 1E-14

int main (int argc, char** argv){

	double pi = 0.0;			//pi greco
	int rank, p;				//rank e numero totale dei processori
	double res_red;				//variabile di ricezione dei punti calcolati da ogni processore
	int *buffer1, *buffer2;			//buffer che contengono i valori di inizio e di fine della parte calcolata da ogni processore
	int start, end;				//variabili per l'estrazione dai rispettivi buffer
	
	double start_time = 0.0, end_time = 0.0;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if(rank == 0){

		if(argc != 1){
			printf("*** ERROR *** : il metodo non richiede parametri di input.\n");
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_COUNT);
		}

		if(p == 1){
			
			//Esecuzione del codice sequenziale
			sequenzialTrap(start_time, end_time, pi);		

	   		MPI_Finalize();
			return 0;
	   	}

	   	printf("-------------------------------------\n");
		printf("PARALLELO START\n");
		printf("-------------------------------------\n");

	   	buffer1 = (int *) malloc (p * sizeof(int));
		buffer2 = (int *) malloc (p * sizeof(int));

		start_time = MPI_Wtime();
		
		//Calcolo della parte da assegnare ad ogni processore
		partCalculator(buffer1, buffer2, p);				
	}

	MPI_Scatter(buffer1, 1, MPI_INT, &start, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(buffer2, 1, MPI_INT, &end, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Calcolo della regola del Trapezoide da parte di ogni processore
	double res = trapezoidRule(start, end);
	
	//Il processore 0 effettua la somma di tutti i risultati ricevuti dagli altri processori
	MPI_Reduce(&res, &res_red, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);		


	if(rank == 0){
		//Il processore 0 calcola il valore di PI
		pi = 4*d*res_red;						

		end_time = MPI_Wtime();
		double exe_time = end_time - start_time;

	   	printf("PI:             %f\nIterazioni: 	%.f\nTempo: 		%.6f\n", pi, N, exe_time);
	   	
	   	free(buffer1);
	   	free(buffer2);

	   	printf("-------------------------------------\n");
		printf("PARALLELO END\n");
		printf("-------------------------------------\n");
	}

	MPI_Finalize();
	return 0;

}

