/*
 * functionsMC.h
 * Contiene tutte le funzioni relative al calcolo del metodo di Monte Carlo
 * Author: Scavone Francesca, 0522500705
*/

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <time.h>

//Il metodo calcola la regola del Trapezio
int monteCarloMethod(int num){
	
	double x, y, z;			//coordinate dei punti 
	int count = 0;			//punti totali nella circonferenza

	srand(time(0));

	for (int i = 0; i < num; i++) {
		x = (double)rand()/RAND_MAX;
		y = (double)rand()/RAND_MAX;
		z = x*x+y*y;

		if (z<=1) count++;
	}

	return count;
}

//Il metodo calcola le parti che devono essere assegnate ad ogni processore.
void partCalculator(int *buff, int proc, long int num){

	int n_iter_red;			//divisione delle iterazioni
	int n_iter_rem;			//resto della divisione

	n_iter_red = num / proc; 
	n_iter_rem = num % proc;

	for (int i = 0; i < proc; ++i){
		buff[i] = (i < n_iter_rem ) ? n_iter_red+1 : n_iter_red;
	}

}

//Il metodo calcola la versione sequenziale
void sequenzialMC(double start_time, double end_time, long double pi, long int n_iter){

	printf("-------------------------------------\n");
	printf("SEQUENZIALE START\n");
	printf("-------------------------------------\n");

	start_time = MPI_Wtime();

	int res = monteCarloMethod(n_iter);

	end_time = MPI_Wtime();

	pi = (double) res / n_iter * 4;

	double exe_time = end_time - start_time;
	   	
   	printf("PI:             %Lf\nIterazioni: 	%li\nTempo: 		%.6f\n", pi, n_iter, exe_time);

	printf("-------------------------------------\n");
	printf("SEQUENZIALE END\n");
	printf("-------------------------------------\n");

}
