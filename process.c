#include<stddef.h>
#include<stdbool.h>
#include<time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include "process.h"
#include "gridHandler.h" //in order to acces the "ind" function
#include "PriorityQueue.h"

static void wait(int offset){
	struct sembuf buf = { offset, -1, 0};
	if((semop(semId, &buf, 1)) == -1){
		fprintf(stderr, "Wait failed\n");
		exit(EXIT_FAILURE);
	}
}

static void signal(int offset){
	struct sembuf buf = { offset, 1,0};
	if((semop(semId, &buf, 1)) == -1){
		fprintf(stderr, "Signal failed\n");
		exit(EXIT_FAILURE);
	}
}

static void sendMessage(myMsg* msg){
	if(msgsnd(qId,msg, sizeof(myMsg) - sizeof(long), 0) < 0){
		fprintf(stderr, "sending a message failed\n");
		exit(EXIT_FAILURE);
	}
}

static void readMessage(long type, int* offset){
    if(msgrcv(qId, offset, sizeof(myMsg) - sizeof(long), type,  0) < 0){
		fprintf(stderr, "reading a message failed\n");
		exit(EXIT_FAILURE);
	}
}

// show the best creature's movements on the terminal
static void showBest(int M, int N, int* bestCreature, int T){
	
}

void listenerProcess(int M, int N, int P, int T){
	printf("type: G to request a new generation\n");
	printf("      M followed by a number to request that number of generations\n");
	printf("      B to display the best creature so far\n");
	printf("      Q to close the program\n");
	while(Offsets->stop != 2){
		char tmp = 'a';
		unsigned int number = 0;
		scanf(%c, &tmp);
		switch (tmp) {
		case 'G' :
			if (Offsets->stop != 1){
				signal(1);
			}
			break;
		case 'M' :
			if (Offsets->stop != 1){
			scanf(%ud, &number);
			for(unsigned int i = 0; i < number; i++){
				signal(1);
			}
			break;
		case 'B' :
			wait(0);
			best = Offsets->best;
			if(best == -1){
				printf("we haven't evaluated any creatures yet\n")
			}
			// we copy the table in order not to block the master process during T seconds
			int bestCreature[T];
			for(int j = 0; j < T; ++j){
				bestCreature[j] = TableGenes[ind(best,j,T)];
			}
			signal(0);
			showBest(M, N, bestCreature, T);
			break;
		case 'Q' :
			if (Offsets->stop == 0){ // we have to close workers and master process
				Offsets->stop = 2;
				// all the workers processes will now close as soon as they get a message
				for(size_t i = 0; i < P; ++i){
					myMsg msg;
					msg.type = 1; // the offset doesn't matter
					sendMessage(&msg);
				}
				// the master can be waiting for a new generation or a message
				signal(1); // we tell him to stop to wait (and to close)
				myMsg msg; // we send a close message to the master
				msg.type = 2;
				msg.offset = -1;
				sendMessage(&msg);
			}else{
				Offsets->stop = 2;
			}
			break;
		
		default: 
			printf("invalid command \n");
		}
	}
	for(int i = 0; i < P+1; ++i){  // we wait untill master + all worker processes are closed
		wait(2);
	}
	// delete the semaphore/message queue, the shared memory has already been flagged for deletion
	semctl(semId,0,IPC_RMID,0));
	msgctl(qId, IPC_RMID, 0);
	exit(EXIT_SUCCES);
}

// computes the score of the creature and possibly replace the best creature's index
static void computeScore(int M, int N, int T, int offset){
	// c'est pas cette fonction qui gère le cas où on aurait un bestScore == 0
}
	
void workerProcess(int M, int N, int T){
	int offset;
	while(stop == 0){
		readMessage(1, &offset);
		if (Offsets->stop != 0){
			break;
		}
		computeScore(M, N, T, offset);
		myMsg msg; // tells the master the math is done
		msg.type = 2;
		msg.offset = offset;
		sendMessage(&msg);
	}
	signal(2); // signals we closed
	exit(EXIT_SUCCESS);
}

// modifies the genes of the creature number index
static void modifyCreature(int index, int p, int T){
	for(int j = 0; j < T; ++j){
		if((rand%99) < p){ // if the move mutates
			int prev = TableGenes[ind(index,j,T)];
			int new = rand()%8;
			while(prev == new){
				new = rand%8;
			}
			TableGenes[ind(index,j,T)] = new;
		}
	}
}

// creates a nex creature at the index
static void createCreature(int index, int T){
	for(int j = 0; j < T; ++j){
		TableGenes[ind(index,j,T)] = rand()%8;
	}
}

void masterProcess(int P, int C, int p, int m, int T){
	MaxHeap* heap = createMaxHeap((size_t) C);
	
	// first generation
	wait(1);
	if(Offsets->stop == 2){
		destroyMaxHeap(heap);
		signal(2);
	}
	for(int i = 0; i < C; ++i){
			createCreature(i, int T);
			myMsg msg; // we send a message to a worker
			msg.type = 1;
			msg.offset = i;
			sendMessage(&msg);
	}
	for(int i = 0; i < C; ++i){
		int offset;
		readMessage(2, &offset);
		if(offset == -1){
			break;
		}
		if(TableScores[offset] == 0.0){
			Offsets->stop = 1;
			for(size_t j = 0; j < P; ++j){
				myMsg msg;
				msg.type = 1; // the offset doesn't matter
				sendMessage(&msg);
			}
			printf("One of the Creatures was able to reach the goal tile\n");
			printf("All you can do now is watch his journey (B) or quit (Q)\n");
			destroyMaxHeap(heap);
			signal(2); // we have to signal we closed
			return;
		}
		insertIndex(offset, heap, TableScores); // we insert the index in the heap
	}
	
	int beginOffset = C * p / 100;
	while(Offsets->stop != 2){
		wait(1);
		if(Offsets->stop == 2){
			break;
		}
		
		wait(0); // we could destroy the creature that was best at a moment
		for(int i = beginOffset; i < C; ++i){
			index = extractIndexForMax(heap, TableScores);
			modifyCreature(index, int p, int T);
			myMsg msg; // we send a message to a worker
			msg.type = 1;
			msg.offset = index;
			sendMessage(&msg);
		}
		signal(0);
		
		for(int i = beginOffset; i < C; ++i){
			int offset;
			readMessage(2, &offset);
			if(offset == -1){
				break;
			}
			if(TableScores[offset] == 0.0){
				Offsets->stop = 1;
				for(size_t j = 0; j < P; ++j){
					myMsg msg;
					msg.type = 1; // the offset doesn't matter
					sendMessage(&msg);
				}
				printf("One of the Creatures was able to reach the goal tile\n");
				printf("All you can do now is watch his journey (B) or quit (Q)\n");
				destroyMaxHeap(heap);
				signal(2); // we have to signal we closed
				return;
			}
			insertIndex(offset, heap, TableScores); // we insert the index in the heap
		}
	}
	destroyMaxHeap(heap);
	signal(2);
	return;
}
