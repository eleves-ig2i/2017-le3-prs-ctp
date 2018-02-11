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

#define NBC 2 // nombre de cabine
#define NBP 4 // nombre de paniers
#define CLIENT_COUNT_MAX 150

#define STATE_DEFAULT 0
#define STATE_WAITING_FOR_PANIER 1
#define STATE_WAITING_FOR_CABINE 2
#define STATE_CHANGING 3
#define STATE_SWIMMING 4
#define STATE_WAITING_FOR_CABINE_AGAIN 5
#define STATE_CHANGING_AGAIN 6
#define STATE_DONE 7

#define MIN_WAIT 10
#define MAX_WAIT 100

#define CHECK(sts,msg) if ((sts) == -1) { perror(msg); exit(sts); }
#define MY_ALEA(min,max) usleep(1000*(rand()%(max-min)+min))


////////////////////////////////////////////////////////////////////////////////
// Structures
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
void displaying(Client ** clients, int totalClientCount, 
		int totalCabineCount, int totalPanierCount, int * go) {
	int i = 0;
	int panierCount = totalPanierCount;
	int cabineCount = totalCabineCount;
	int attentes = 0;
	int baignades = 0;
	while(*go) {
				usleep(50*1000);
		panierCount = totalPanierCount;
		cabineCount = totalCabineCount;
		attentes = 0;
		baignades = 0;
		printf("%10i |", (int)clock());
		for(i = 0 ; i < totalClientCount ; i++) {
			switch(clients[i]->stateId) {
				case STATE_DEFAULT : {
					printf(".");
					break;
				}
				case STATE_WAITING_FOR_PANIER : {
					attentes++;
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_WAITING_FOR_CABINE : {
					attentes++;
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_CHANGING : {
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_SWIMMING : {
					baignades++;
					printf("%d", clients[i]->stateId);
					break;
				}
				case STATE_WAITING_FOR_CABINE_AGAIN : {
					attentes++;
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
		printf("| P %d/%1i", panierCount, totalPanierCount);
		printf(" | C %d/%1i", cabineCount, totalCabineCount);
		printf(" | A %2i", attentes);
		printf(" | B %2i", baignades);
		printf(" |\n");
	}
}


////////////////////////////////////////////////////////////////////////////////
// -
////////////////////////////////////////////////////////////////////////////////
void * clientFunction(void * client_) {

	Client * client = (Client *)client_;
	MY_ALEA(MIN_WAIT, MAX_WAIT*client->id);
	client->isInCabine = 0;
	client->hasPanier = 0;

	client->stateId = STATE_WAITING_FOR_PANIER;
	sem_wait(client->paniers);
	client->hasPanier = 1;
	client->stateId = STATE_WAITING_FOR_CABINE;
	sem_wait(client->cabines);
	client->isInCabine = 1;
	client->stateId = STATE_CHANGING;
	MY_ALEA(MIN_WAIT,MAX_WAIT);
	sem_post(client->cabines);
	client->isInCabine = 0;
	client->stateId = STATE_SWIMMING;
	MY_ALEA(MIN_WAIT,MAX_WAIT);
	client->stateId = STATE_WAITING_FOR_CABINE_AGAIN;
	sem_wait(client->cabines);
	client->isInCabine = 1;
	client->stateId = STATE_CHANGING_AGAIN;
	MY_ALEA(MIN_WAIT,MAX_WAIT);
	sem_post(client->cabines);
	client->isInCabine = 0;
	sem_post(client->paniers);
	client->hasPanier = 0;
	client->stateId = STATE_DONE;

	pthread_exit(NULL);
}

void initSemaphores(sem_t * cabines, sem_t * paniers, 
		int totalCabineCount, int totalPanierCount) {
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

	srand(time(NULL));
	int totalCabineCount = NBC;
	int totalPanierCount = NBP;
	int totalClientCount = CLIENT_COUNT_MAX;
	int go = 1;

	printf("\
********************************\n\
* PRS - CTP V2 - Loïc Bourgois *\n\
********************************\n");
	printf("\
Légende \n\
	. - Etat initial\n\
	1 - En attente d'un panier\n\
	2 - En attente d'une cabine\n\
	3 - Se change\n\
	4 - Se baigne\n\
	5 - En attente d'une cabine après baignade\n\
	6 - Se change après baignade\n\
	  - Terminé\n\
	P - Paniers disponibles\n\
	C - Cabines disponibles\n\
	A - Clients en attentes\n\
	B - Clients en baignade\n\n");

	//sem_t lock;
	sem_t cabines;
	sem_t paniers;
	pthread_t clientThreads[totalClientCount];
	Client ** clients = malloc(sizeof(Client)*totalClientCount);

	initSemaphores(&cabines, &paniers, totalCabineCount, totalPanierCount);
	startThreads(clientThreads, &cabines, &paniers, totalClientCount, clients);

	displaying(clients, totalClientCount, totalCabineCount, totalPanierCount, &go);

	printf("Tout est bien qui fini bien\n");

	return 1;
}
