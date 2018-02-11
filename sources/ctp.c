////////////////////////////////////////////////////////////////////////////////
//	Pour lancer le client : 
//			clear ; make clean ; make ; ./bin/client 127.0.0.1 3000
////////////////////////////////////////////////////////////////////////////////
//#include "constants.h"
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

//
#define NBC 2 // nombre de cabine
#define NBP 4 // nombre de paniers
//
#define STATE_DEFAULT 0
#define STATE_WAITING_FOR_PANIER 1
#define STATE_WAITING_FOR_CABINE 2
#define STATE_CHANGING 3
#define STATE_SWIMMING 4
#define STATE_WAITING_FOR_CABINE_AGAIN 5
#define STATE_CHANGING_AGAIN 6
#define STATE_DONE 7
////////////////////////////////////////////////////////////////////////////////
// Structure
////////////////////////////////////////////////////////////////////////////////
typedef struct {
		int id;
		int stateId;
		sem_t * paniers;
		sem_t * cabines;
	int hasPanier;
	int isInCabine;
} Client;

typedef struct {
		int id;
		int durationMilli;
	char bob[8]; 
} State;

////////////////////////////////////////////////////////////////////////////////
// Affichage
////////////////////////////////////////////////////////////////////////////////
void displaying(int * cabineCount, int * panierCount) {
	int go = 1;	
	while(go) {
				usleep(500*1000);
		printf("\n\n------------------------------------------\n\n");
		printf(" Cabines dispo : %d / %d\n", *cabineCount, *cabineCount);
		printf(" Paniers dispo : %d / %d\n", *panierCount, *panierCount);
		printf(" Client wianitng : %d / %d\n", 0, 0);
		printf(" Client swimming : %d / %d\n", 0, 0);
	}
}

void displaying2(Client ** clients, int totalClientCount, 
		int totalCabineCount, int totalPanierCount) {
	int go = 1;	
	int i = 0;
	int panierCount = totalPanierCount;
	int cabineCount = totalCabineCount;
	while(go) {
				usleep(20*1000);
		panierCount = totalPanierCount;
		cabineCount = totalCabineCount;
		for(i = 0 ; i < totalClientCount ; i++) {
			switch(clients[i]->stateId) {
				case STATE_DEFAULT : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_WAITING_FOR_PANIER : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_WAITING_FOR_CABINE : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_CHANGING : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_SWIMMING : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_WAITING_FOR_CABINE_AGAIN : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_CHANGING_AGAIN : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_DONE : {
					printf(" ");
					break;
				}
				default: {
					printf("ZZZ %d ZZZ", clients[i]->id);
					break;				
				}
			}
			panierCount -= clients[i]->hasPanier;
			cabineCount -= clients[i]->isInCabine;
		}
		printf("|	paniers dispos %d/%d", panierCount, totalPanierCount);
		printf("	|	cabines dispos %d/%d", cabineCount, totalCabineCount);
		printf("	|\n");
	}
}


////////////////////////////////////////////////////////////////////////////////
// -
////////////////////////////////////////////////////////////////////////////////
void * clientFunction(void * client_) {
	// Init
	Client * client = (Client *)client_;
	client->isInCabine = 0;
	client->hasPanier = 0;

	//

	client->stateId = STATE_WAITING_FOR_CABINE;
	sem_wait(client->cabines);
	client->isInCabine = 1;
	
	client->stateId = STATE_WAITING_FOR_PANIER;
	sem_wait(client->paniers);
	client->hasPanier = 1;

	client->stateId = STATE_CHANGING;
	usleep(50*1000);
	sem_post(client->cabines);
	client->isInCabine = 0;

	client->stateId = STATE_SWIMMING;
	usleep(50*1000);
	client->stateId = STATE_WAITING_FOR_CABINE_AGAIN;
	sem_wait(client->cabines);
	client->isInCabine = 1;

	client->stateId = STATE_CHANGING;
	usleep(50*1000);

	sem_post(client->cabines);
	client->isInCabine = 0;
	sem_post(client->paniers);
	client->hasPanier = 0;

	client->stateId = STATE_DONE;
	return 0;
}

void initSemaphores(sem_t * cabines, sem_t * paniers, 
		int totalCabineCount, int totalPanierCount) {
	/*int i;
		for(i = 0 ; i < totalCabineCount ; i++) {
				sem_init(&cabines[i], 0, 1);
		}
		for(i = 0 ; i < totalPanierCount ; i++) {
				sem_init(&paniers[i], 0, 1);
		}*/
	sem_init(cabines, 0, totalCabineCount);
	sem_init(paniers, 0, totalPanierCount);
}

void startThreads(pthread_t * threads, sem_t * cabines, sem_t * paniers,
		int totalClientCount, Client ** clients) {
	int i = 0;
	for(i = 0 ; i < totalClientCount ; i++) {
		// Init
		clients[i] = malloc(sizeof(Client));
		clients[i]->id = i;
		clients[i]->stateId = STATE_DEFAULT;
		clients[i]->cabines = cabines;
		clients[i]->paniers = paniers;
		clients[i]->isInCabine = 0;
		clients[i]->hasPanier = 0;
		// Launch thread
		pthread_create(&threads[i], 
			NULL, 
			clientFunction, 
			(void *)(clients[i]));
	}
}

////////////////////////////////////////////////////////////////////////////////
// Début du programme
////////////////////////////////////////////////////////////////////////////////
int main (int c, char**v) {
	// Init
	int totalCabineCount = NBC;
	int totalPanierCount = NBP;
	int totalClientCount = 20;
	//
	printf("\
*****************************\n\
* PRS - CTP - Loïc Bourgois *\n\
*****************************\n");
	printf("\
Légende \n\
	0 - Etat initial\n\
	1 - En attente d'un panier\n\
	2 - En attente d'une cabine\n\
	3 - Se change\n\
	4 - Se baigne\n\
	5 - En attente d'une cabine après baignade\n\
	6 - Se change après baignade\n\
		- Terminé\n\n");
	//

	//sem_t lock;
	sem_t cabines;
	sem_t paniers;
	pthread_t clientThreads[totalClientCount];
	Client ** clients = malloc(sizeof(Client)*totalClientCount);

	initSemaphores(&cabines, &paniers, totalCabineCount, totalPanierCount);
	startThreads(clientThreads, &cabines, &paniers, totalClientCount, clients);


	//
	//displaying(&totalCabineCount, &totalPanierCount);
	displaying2(clients, totalClientCount, totalCabineCount, totalPanierCount);

	return 1;
}
