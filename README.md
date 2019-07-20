# PI Greco 

## Programmazione Concorrente, Parallela e su Cloud
### Università degli Studi di Salerno
#### *Anno Accademico 2018/2019*

**Professore:** _Vittorio Scarano_
**Dottore:** _Carmine Spagnuolo_
**Studente:** _Scavone Francesca_

---

Il seguente progetto presenta una soluzione per determinare il valore di π attraverso due metodi:

* **Regola del Trapezoide** 
* **Metodo di Monte Carlo**

## Esecuzione

All'interno della directory ottenuta è possibile ritrovare i due file contenenti il codice sorgente. Ognuno di esso rappresenta in modo chiaro il metodo che vanno ad implementare: se si vuole eseguire il metodo di Monte Carlo si aprirà il file *PiMonteCarlo.c*; se si vuole eseguire la regola del Trapezoide si aprirà il file *PiTrapezoidRule.c*

Una volta definito il metodo da utilizzare è possibile compilare il *file.c* su cloud AWS con il seguente comando:
```
mpicc -fopenmp file.c -o file
```
Successivamente bisognerà inviare il *file* eseguibile su tutte le altre macchine, in modo tale da poter eseguire il codice in maniera parallela. 
```
scp file pcpc@privateIp:~
```
A questo punto è possibile eseguire il programma. Entrambi i sorgenti contengono sia la versione sequenziale che quella parallela: attraverso l'inserimento di un solo processore sarà possibile eseguire la versione sequenziale.

Ad esempio: 
```
mpirun -np 1 --hostfile slavesfile ./file
```
Per la versione che implementa il metodo di Monte Carlo è necessario inserire, nella riga di comando, anche il numero di iterazioni che si intende eseguire. Ad esempio se si vuole eseguire il metodo con *32 processori* e per un totale di *100.000.000* iterazioni si dovrà compliare nel seguente modo:
```
mpirun -np 32 --hostfile slavesfile ./MonteCarlo 100000000
```

## Implementazione del metodo di Monte Carlo
Il metodo di Monte Carlo si basa sull’area di un cerchio di raggio unitario iscritto in un quadrato: dato un cerchio di raggio 1, esso può essere inscritto in un quadrato di raggio 2. Se si prende in considerazione solamente un quarto del quadrato è possibile ottenere un quadrato con area 1 e uno spicchio di cerchio con area pari a π/4. Con la generazione di N numeri casuali all'interno del quadrato, il numero totale di punti M che cadono all'interno del cerchio diviso il numero totale di numeri generati dovrà approssimare l'area del cerchio, ovvero π/4. Quindi varrà che π = 4 * M/N.

### Sviluppo
Il numero di iterazioni vengono prelevate da riga di comando: il numero massimo di iterazioni, a causa della codifica utilizzata, è di massimo *2.000.000.000* iterazioni. Per poter calcolare le parti da assegnare ad ogni processo, si è prima di tutto diviso il numero di iterazioni totali per il numero di processori. Nel caso in cui tale divisione presentava resto, ai processori è stato assegnato un valore in più fino a quando non è stato consumato.

```
void partCalculator(int *buff, int proc, int num){

	int n_iter_red;						//divisione delle iterazioni
	int n_iter_rem;						//resto della divisione

	n_iter_red = num / proc; 
	n_iter_rem = num % proc;

	for (int i = 0; i < proc; ++i){
		buff[i] = (i < n_iter_rem ) ? n_iter_red+1 : n_iter_red;
	}

}
```

La comunicazione del numero di punti che ogni processore deve calcolare avviene attraverso la funzione **MPI_Scatter**: il master invierà ai propri slave la parte del buffer che gli spetta e ogni slave salverà nella variabile *iteration* ciò che ha ricevuto. A questo punto master e slave sapranno il numero di iterazioni che dovranno eseguire e possono applicare il metodo di Monte Carlo.

Attraverso la funzione di **MPI_Reduce** tutti i risultati vengono sommati e mantenuti all'interno della variabile *count_red* presente nel master. Ottenuti i risultati da tutti i processori, può finalmente calcolare il valore di π e liberare la memoria allocata.

## Implementazione della regola del Trapeziode
Nel metodo del Trapezoide il valore di π può essere calcolato attraverso l'integrale di *f(x)* definito tra 0 e 1, dove *f(x)* = 4/(1+x^2). 

### Sviluppo
Per poter calcolare le parti da assegnare ad ogni processo, come nel caso precedente, si è prima di tutto diviso la porzione di calcolo (il punto iniziale e il punto finale della partizione) da assegnare ad ogni processo. Nel caso in cui tale divisione presentava resto, ai processori è stato assegnato un valore in più fino a quando non è stato consumato.
```
void partCalculator(int* buff1, int* buff2, int proc){

	int n_iter_red;						//divisione delle iterazioni
	int n_iter_rem;						//resto della divisione

	n_iter_red = N / proc;
	n_iter_rem = (int) N % proc;

	int part = 0;

	for(int i = 0; i < proc; ++i){
		buff1[i] = part;
		part += (i < n_iter_rem ) ? n_iter_red+1 : n_iter_red;
		buff2[i] = part;
	}
	
}
```
La comunicazione delle partizioni che ogni processore deve calcolare avviene attraverso la funzione **MPI_Scatter**: il master invierà ai propri slave la parte dei due buffer che gli spetta e ogni slave salverà nella variabile *start* ed *end* ciò che ha ricevuto. A questo punto master e slave sapranno la porzione che dovranno calcolare e possono applicare la regola del Trapezoide.

Attraverso la funzione di **MPI_Reduce** tutti i risultati vengono sommati e mantenuti all'interno della variabile *res_red* presente nel master. Ottenuti i risultati da tutti i processori, può finalmente calcolare il valore di π e liberare la memoria allocata.

## Risultati del benchmarking

I test sono stati realizzati attraverso otto istante di tipo **m4.xlarge**, ognuna costituita da 4 VCPU, di Amazon Web Services con *StarCluster-Ubuntu_12.04-x86_64-hvm - ami-52a0c53b*. Essi sono stati effettuati su tutte le istanze disponibili (da 1 a 32 VCPU) e ripetuti cinque volte per ogni istanza. Nei risultati viene, quindi, riportata una media dei cinque esperimenti compiuti.

In entrambi i metodi il tempo di esecuzione racchiude sia il calcolo della divisione delle parti che ogni processore deve calcolare che l'effettivo calcolo del metodo. Questa scelta è stata effettuata per comprendere se l'overhead risultante dal calcolo delle parti viene compensato dalla esecuzione parallela. 

La rappresentazione dei tempi è rappresentata in **secondi** ed è stata utilizzata l'apposita funzione di MPI, ```MPI_Wtime()```, per la loro raccolta.

### Strong Scaling
L'obiettivo dello *strong scaling* è quello di ridurre al minimo il tempo necessario alla produzione della soluzione per un determinato problema: si deve determinare quello che è uno *sweet-spot* per completare il calcolo in tempo ragionevole in modo tale da non sprecare cicli macchina a causa del sovraccarico parallelo.

***Calcolo dello speedup***: nello strong scaling il calcolo dello speedup è dettato dalla formula **t1 / (N x tN)**, dove *t1* è il tempo impiegato per completare un'unità di lavoro con un unico elemento di elaborazione e *tN* è il tempo impiegato per completare la medesima unità di lavoro con *N* unità di elaborazione. Per una migliore lettura dei risultati, il risultato di tale formula è stato espresso in percentuale. 

* ***Regola del Trapezoide***

I test sono stati effettuati, per ognuna delle istanze pensate, su un totale di *10.000.000* iterazioni: tale numero è fissato nella definizione del problema.

Di seguito vengono riportati i risultati, in media, delle cinque esecuzioni fatte su ogni tipologia di istanza sia in una tabella che in un grafico:

| N. Iterazioni	| N. Core | Tempo (s) 	| Speedup 	| N. Iterazioni	| N. Core | Tempo (s) 	| Speedup 	|
| :------------:| :-----: | :---------: | :-----------:	| :------------:| :-----: | :---------: | :-----------:	|
| 10.000.000	| 1 	  | 0,0334s	| x		| 10.000.000	| 17 	  | 0,0381s	| 5,16%		|
| 10.000.000	| 2	  | 0,0168s	| 99,28% 	| 10.000.000	| 18 	  | 0,0392s	| 4,74%		|
| 10.000.000	| 3	  | 0,0184s	| 60,41% 	| 10.000.000	| 19 	  | 0,0396s	| 4,44%		|
| 10.000.000	| 4  	  | 0,0143s	| 58,32%	| 10.000.000	| 20 	  | 0,0381s	| 4,38%		|
| 10.000.000	| 5	  | 0,0308s	| 21,70% 	| 10.000.000	| 21 	  | 0,0370s	| 4,31%		|
| 10.000.000	| 6	  | 0,0382s	| 14,60% 	| 10.000.000	| 22 	  | 0,0349s	| 4,35%		|
| 10.000.000	| 7	  | 0,0431s	| 11,07% 	| 10.000.000	| 23 	  | 0,0358s	| 4,05%		|
| 10.000.000	| 8	  | 0,0519s	| 8,06%		| 10.000.000	| 24	  | 0,0400s	| 3,48%		|
| 10.000.000	| 9	  | 0,0637s	| 5,83% 	| 10.000.000	| 25 	  | 0,0371s	| 3,60%		|
| 10.000.000	| 10	  | 0,0671s	| 4,98%		| 10.000.000	| 26 	  | 0,0410s	| 3,14%		|
| 10.000.000	| 11	  | 0,0299s	| 10,14% 	| 10.000.000	| 27 	  | 0,0351s	| 3,52%		|
| 10.000.000	| 12	  | 0,0319s	| 8,71% 	| 10.000.000	| 28	  | 0,0334s	| 3,78%		|
| 10.000.000	| 13	  | 0,0331s	| 7,76% 	| 10.000.000	| 29 	  | 0,0378s	| 3,05%		|
| 10.000.000	| 14	  | 0,0332s	| 7,19% 	| 10.000.000	| 30 	  | 0,0378s	| 2,95%		|
| 10.000.000	| 15	  | 0,0298s	| 7,48% 	| 10.000.000	| 31 	  | 0,0350s	| 3,08%		|
| 10.000.000	| 16	  | 0,0288s	| 7,25%		| 10.000.000	| 32 	  | 0,0334s	| 2,72%		|


<img src="https://i.imgur.com/RW8k77X.jpg" title="TRAP_strong"/>

Dai risultati ottenuti si può determinare che lo *sweet-spot* ideale per questa tipologia di algoritmo è pari a **2 processori**, in cui lo speedup è quasi del 100%, nonostante si abbiamo ottime prestazioni fino a 4 processori: con l'aumentare dei processori vi è un degrado delle prestazioni. Questo perché il costo per effettuare la divisione del problema tra tutti i processori e la comunicazione tra di essi impiega più tempo rispetto all'effettiva risoluzione del problema: quindi l'overhead causato da tali operazioni non viene compensato dall'esecuzione parallela.

Il valore di π ottenuto durante le esecuzioni del metodo è pari a *3,141593*, molto vicino al vero valore di π=*3,141592*.

* ***Metodo di Monte Carlo***

I test sono stati effettuati, per ognuna delle istanze in esame, su un totale di *1.000.000.000* iterazioni.

Di seguito vengono riportati i risultati, in media, delle cinque esecuzioni fatte su ogni tipologia di istanza sia in una tabella che in un grafico:

| N. Iterazioni	| N. Core | Tempo (s) 	| π Value  | Speedup | N. Iterazioni | N. Core 	| Tempo (s) | π Value  | Speedup | 
| :-----------: | :-----: | :---------: | :------: | :-----: | :-----------: | :------:	| :-------: | :------: | :-----: |
| 1.000.000.000	| 1 	  | 42,9785s	| 3,141611 | x 	     | 1.000.000.000 | 17	| 3,9860s   | 3,1418728| 63,43%	 |
| 1.000.000.000	| 2	  | 21,4951s	| 3,14163  | 99,97%  | 1.000.000.000 | 18	| 3,9951s   | 3,141755 | 59,77%	 |
| 1.000.000.000	| 3	  | 19,4684s	| 3,1415358| 73,59%  | 1.000.000.000 | 19	| 3,5119s   | 3,1415708| 64,41%	 |
| 1.000.000.000	| 4  	  | 16,0919s	| 3,141562 | 66,77%  | 1.000.000.000 | 20	| 3,3624s   | 3,1415242| 63,91%  |
| 1.000.000.000	| 5	  | 13,4458s	| 3,1415916| 63,93%  | 1.000.000.000 | 21	| 3,4510s   | 3,1413972| 59,30%	 |
| 1.000.000.000	| 6	  | 10,7571s	| 3,1416058| 66,59%  | 1.000.000.000 | 22	| 3,0782s   | 3,1415546| 63,46%	 |
| 1.000.000.000	| 7	  | 9,5695s	| 3,1416278| 64,16%  | 1.000.000.000 | 23	| 3,0223s   | 3,1414918| 61,83%	 |
| 1.000.000.000	| 8	  | 8,1022s	| 3,1415866| 66,31%  | 1.000.000.000 | 24	| 2,7982s   | 3,1416364| 64,00%	 |
| 1.000.000.000	| 9	  | 7,6495s	| 3,1416192| 62,43%  | 1.000.000.000 | 25	| 2,8664s   | 3,1416822| 59,97%  |
| 1.000.000.000	| 10	  | 6,4810s	| 3,1415066| 66,31%  | 1.000.000.000 | 26	| 2,7493s   | 3,1416522| 60,12%	 |
| 1.000.000.000	| 11	  | 6,2517s	| 3,1416296| 62,50%  | 1.000.000.000 | 27	| 2,6398s   | 3,1415876| 60,30%	 |
| 1.000.000.000	| 12	  | 5,8607s	| 3,1417202| 61,11%  | 1.000.000.000 | 28	| 2,5718s   | 3,1414754| 59,68%	 |
| 1.000.000.000	| 13	  | 5,4714s	| 3,141572 | 60,42%  | 1.000.000.000 | 29	| 2,5323s   | 3,1417464| 58,52%	 |
| 1.000.000.000	| 14	  | 4,6684s	| 3,1415524| 65,76%  | 1.000.000.000 | 30	| 2,3431s   | 3,141551 | 61,14%	 |
| 1.000.000.000	| 15	  | 4,5818s	| 3,1415184| 62,53%  | 1.000.000.000 | 31	| 42,9785s  | 3,141692 | 58,89%	 |
| 1.000.000.000	| 16	  | 4,4287s	| 3,141626 | 60,65%  | 1.000.000.000 | 32	| 2,3293s   | 3,1415886| 57,66%  | 

<img src="https://i.imgur.com/SyJA4Wm.jpg" title="MC_strong1"/>

Dai risultati ottenuti si può determinare che lo *sweet-spot* ideale per questa tipologia di algoritmo è pari a **2 processori**, in cui lo speedup è quasi del 100%: nonostante ciò, in linea generale, l'algoritmo ha un'evidente miglioramento con l'aumentare dei core utilizzati per la parallelizzazione. Ovviamente, oltre una certa soglia, lo speedup tende a variare intorno allo stesso range di valori nonostante l'aumento dei core: questo è dovuto all'overhead causato dalla comunicazione e dalla divisione dei compiti. In questo caso potrebbe essere ragionevole utilizzare fino ad un massimo di *16 processori* in cui lo speedup minimo è di circa del 60%.

Il valore medio di π ottenuto tra tutte le esecuzioni del metodo è pari a *3,1416023*, molto vicino al vero valore di π=*3,141592*.

### I due metodi a confronto
Per poter effettuare un confronto equo tra i due metodi, è stato effettuato un secondo test *strong scaling* sul metodo di Monte Carlo in cui il numero di iterazioni totali coincidevano con quelle fissate nella definizione della regola del Trapezoide. Di seguito vengono riportati i risultati ottenuti:

| N. Iterazioni	| N. Core | Tempo (s) 	| π Value | Speedup 	| N. Iterazioni	| N. Core | Tempo (s) 	| π Value | Speedup 	| 
| :------------:| :-----: | :---------: | :-----: | :---------: | :------------:| :-----: | :---------: | :-----: | :---------: |
| 10.000.000	| 1 	  | 0,4300s	| 3,141567| x		| 10.000.000	| 17 	  | 0,0738s	| 3,142105| 34,26%	|
| 10.000.000	| 2	  | 0,2230s	| 3,141247| 99,39% 	| 10.000.000	| 18 	  | 0,0679s	| 3,140682| 35,17%	|
| 10.000.000	| 3	  | 0,2088s	| 3,140914| 68,66% 	| 10.000.000	| 19 	  | 0,0705s	| 3,143572| 32,07%	|
| 10.000.000	| 4  	  | 0,1756s	| 3,140672| 61,23%	| 10.000.000	| 20	  | 0,0732s	| 3,140348| 29,34%	|
| 10.000.000	| 5	  | 0,1420s	| 3,141425| 60,57% 	| 10.000.000	| 21 	  | 0,0657s	| 3,141635| 31,14%	|
| 10.000.000	| 6	  | 0,1340s	| 3,141915| 53,47% 	| 10.000.000	| 22 	  | 0,0666s 	| 3,141530| 29,32%	|
| 10.000.000	| 7	  | 0,1230s	| 3,141934| 49,93% 	| 10.000.000	| 23 	  | 0,0623s	| 3,140805| 30,00%	|
| 10.000.000	| 8	  | 0,1270s	| 3,142391| 42,30%	| 10.000.000	| 24	  | 0,0620s	| 3,140679| 28,90%	|
| 10.000.000	| 9	  | 0,1128s	| 3,141759| 42,34% 	| 10.000.000	| 25	  | 0,0619s	| 3,140468| 27,79%	|
| 10.000.000	| 10	  | 0,1123s	| 3,142392| 38,27%	| 10.000.000	| 26 	  | 0,0647s	| 3,141706| 25,54%	|
| 10.000.000	| 11	  | 0,0894s	| 3,141597| 43,73% 	| 10.000.000	| 27 	  | 0,0625s	| 3,141314| 25,45%	|
| 10.000.000	| 12	  | 0,0794s	| 3,140874| 45,09% 	| 10.000.000	| 28 	  | 0,0599s	| 3,141674| 25,62%	|
| 10.000.000	| 13	  | 0,0773s	| 3,140853| 42,77% 	| 10.000.000	| 29 	  | 0,0629s	| 3,141987| 23,55%	|
| 10.000.000	| 14	  | 0,0755s	| 3,139994| 40,65% 	| 10.000.000	| 30 	  | 0,0592s	| 3,143509| 24,21%	|
| 10.000.000	| 15	  | 0,0713s	| 3,141022| 40,20% 	| 10.000.000	| 31 	  | 0,0593s	| 3,142408| 23,37%	|
| 10.000.000	| 16	  | 0,0696s	| 3,142124| 38,59%	| 10.000.000	| 32	  | 0,0592s	| 3,142453| 22,68%	| 

<img src="https://i.imgur.com/C8WaPPQ.jpg" title="MC_strong2"/>

Attraverso questa nuova esecuzione dello *strong scaling* risulta ancora più evidente come, da una determinata soglia in poi, lo speedup tende a stabilizzarsi attorno ad un range di valori (dal 35% al 22%) e che, quindi, non sono necessari un numero di core elevato per ottenere buone prestazioni. 

* ***Approssimazione***

L'approssimazione ottenuta attraverso il metodo di Monte Carlo, in media tra tutte le esecuzioni, è pari a *3,141549* mentre per la regola del Trapezoide è pari a *3,141593*: l'ultimo metodo risulta essere quello che approssima meglio il valore di π=*3,141592*. 

* ***Tempo di esecuzione***

<img src="https://i.imgur.com/YR8cmyD.jpg" title="TRAP_vs_MC"/>

I tempi d'esecuzione dell'algoritmo che implementa la regola del Trapezio risultano essere nettamente minori rispetto a quelli ottenuti dall'algoritmo che implementa il metodo di Monte Carlo.

* ***Conclusioni*** Dai confronti è, quindi, evidente che tra i due metodi risulta essere preferibile quello dello Trapezoide: fornisce una migliore approssimazione in un tempo minore.

### Weak Scaling
L'obiettivo del *weak scaling* è quello di determinare velocemente l'efficienza di un dato algoritmo diminuisce quando il numero di processero aumenta ma la dimensione del problema è fissata. La dimensione del problema aumenta linearmente assieme al numero di processori, mantendo invariata la quantità di lavoro per ogni processore. Lo scopo è quindi di determinare la risposta del programma su problemi più grandi.

***Calcolo dello speedup***: nel weak scaling il calcolo dello speedup è dettato dalla formula **t1 / tN**, dove *t1* è il tempo impiegato per completare un'unità di lavoro con un unico elemento di elaborazione e *tN* è il tempo impiegato per completare *N* delle medesime unità di lavoro con *N* unità di elaborazione. Per una migliore lettura dei risultati, il risultato di tale formula è stato espresso in percentuale. 

* ***Regola del Trapezoide***

Essendo il numero di iterazioni fissate nella definizione del problema, non è stato possibile effettuare questo tipo di test sul sorgente che implementava tale metodo.

* ***Metodo di Monte Carlo***

I test sono stati effettuati partendo da una istanza per un core con un numero di iterazioni pari a *62.500.000* e aumentando l'input fino ad arrivare ai 32 core con un totale di *2.000.000.000* iterazioni.

Di seguito vengono riportati i risultati, in media, delle cinque esecuzioni fatte su ogni tipologia di istanza sia in una tabella che in un grafico:

| N. Iterazioni	| N. Core | Tempo (s) 	| π Value  | Speedup 	| N. Iterazioni	| N. Core | Tempo (s) 	| π Value  | Speedup 	| 
| :------------:| :-----: | :---------: | :------: | :--------: | :------------:| :-----: | :---------: | :------: | :--------: |
| 62.500.000	| 1 	  | 2,6833s	| 3,141585 | x		| 1.062.500.000	| 17 	  | 4,1714s	| 3,141667 | 64,33%	|
| 125.000.000	| 2	  | 2,6866s	| 3,141561 | 99,87% 	| 1.125.000.000	| 18	  | 4,2770s	| 3,141452 | 62,74% 	|
| 187.500.000	| 3	  | 3,8750s	| 3,1417   | 69,25% 	| 1.187.500.000	| 19	  | 4,2775s	| 3,141608 | 62,73% 	|
| 250.000.000	| 4  	  | 4,1293s	| 3,141649 | 64,98%	| 1.250.000.000	| 20	  | 4,0944s	| 3,141533 | 65,54%	| 
| 312.500.000	| 5	  | 4,1028s	| 3,141568 | 65,40% 	| 1.312.500.000	| 21	  | 4,3211s	| 3,141597 | 62,10% 	|
| 375.000.000	| 6	  | 4,0621s	| 3,141603 | 66,06% 	| 1.375.000.000	| 22	  | 4,7056s	| 3,141542 | 57,02% 	|
| 437.500.000	| 7	  | 4,0559s	| 3,141667 | 66,16% 	| 1.437.500.000	| 23	  | 4,5387s	| 3,141523 | 59,12% 	|
| 500.000.000	| 8	  | 4,0645s	| 3,141513 | 66,02%	| 1.500.000.000	| 24	  | 3,9879s	| 3,141538 | 67,29%	|
| 562.500.000	| 9	  | 4,0620s	| 3,141521 | 66,06% 	| 1.562.500.000	| 25	  | 4,3840s	| 3,141604 | 61,21%	|
| 625.000.000	| 10	  | 4,1699s	| 3,14163  | 64,35%	| 1.625.000.000	| 26	  | 4,6747s	| 3,141588 | 57,40%	|
| 687.500.000	| 11	  | 4,0517s	| 3,141672 | 66,23% 	| 1.687.500.000	| 27	  | 4,7375s	| 3,141538 | 56,64% 	|
| 750.000.000	| 12	  | 4,1538s	| 3,141435 | 64,60% 	| 1.750.000.000	| 28	  | 4,4032s	| 3,141583 | 60,94% 	|
| 812.500.000	| 13	  | 4,3252s	| 3,141753 | 62,04% 	| 1.812.500.000	| 29	  | 4,5563s	| 3,141522 | 58,89% 	|
| 875.000.000	| 14	  | 4,0648s	| 3,141582 | 66,01% 	| 1.875.000.000	| 30	  | 4,1520s	| 3,141515 | 64,63% 	|
| 937.500.000	| 15	  | 4,4210s	| 3,141568 | 60,70% 	| 1.937.500.000	| 31	  | 4,4869s	| 3,141486 | 59,80% 	|
| 1.000.000.000	| 16	  | 4,4313s	| 3,141618 | 60,55%	| 2.000.000.000	| 32	  | 4,5643s	| 3,141568 | 58,78%	| 

<img src="https://i.imgur.com/PwTgOYT.jpg" title="MC_weak"/>

Dai risultati è possibile notare, così come emerso dallo strong scaling, che dai 4 processori in poi le prestazioni degradano di quasi il doppio, a causa dell'overhead di comunicazione. 
