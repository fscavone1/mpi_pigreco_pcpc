/*
 * functionsTrap.h
 * Contiene tutte le funzioni relative al calcolo della regola del Trapezoide
 * Author: Scavone Francesca, 0522500705
*/


#include <stdio.h>
#include <math.h>
#include "mpi.h"
#include <stdlib.h>

#define N 1E7
#define d 1E-7
#define d2 1E-14

//Il metodo calcola la regola del Trapezio
double trapezoidRule(int start, int end){

	double result = 0.0, x2 = 0.0;

	for (int i = start; i < end; i++){
		x2 = d2 * i * i;
		result += 1.0 / (1.0 + x2);
	    }

    	return result;
}

//Il metodo calcola le parti che devono essere assegnate ad ogni processore, calcolando l'indice iniziale e l'indice finale.
void partCalculator(int* buff1, int* buff2, int proc){

	int n_iter_red;			//divisione delle iterazioni
	int n_iter_rem;			//resto della divisione

	n_iter_red = N / proc;
	n_iter_rem = (int) N % proc;

	int part = 0;

	for(int i = 0; i < proc; ++i){
		buff1[i] = part;
		part += (i < n_iter_rem ) ? n_iter_red+1 : n_iter_red;
		buff2[i] = part;
	}
	
}

//Il metodo calcola la versione sequenziale
void sequenzialTrap(double start_time, double end_time, double pi){

	printf("-------------------------------------\n");
	printf("SEQUENZIALE START\n");
	printf("-------------------------------------\n");

	start_time = MPI_Wtime();

	double res = trapezoidRule(0, (int) N);

	end_time = MPI_Wtime();

	pi = 4 * d * res;

	double exe_time = end_time - start_time;
	   	
	printf("PI:             %f\nIterazioni: 	%.f\nTempo: 		%.6f\n", pi, N, exe_time);

	printf("-------------------------------------\n");
	printf("SEQUENZIALE END\n");
	printf("-------------------------------------\n");

}
