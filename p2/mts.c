/**
 * Parm Johal
 * V00787710
 * CSC 360
 * Assignment p2b
 * mts.c
 *
 * References:
 * -------------------------
 * CSC 360 tutorial slides 5-7 + sample code
 * https://github.com/MichaelReiter/CSC360/blob/5568dbf99064f7691402ce6a12bde7761f405dff/Assignment%202/mfs.c
 *
 * */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

typedef struct train {
	int id;
	char direction;
	float load_time;
	float cross_time;
	int is_executed; //train finished = 1; train not finished = 0
}train;


train *low_eastQ[100];
int qlength_loweast = 0;
train *high_eastQ[100];
int qlength_higheast = 0;
train *low_westQ[100];
int qlength_lowwest = 0;
train *high_westQ[100];
int qlength_highwest = 0;
train trains[100];

pthread_t threads[100];
pthread_mutex_t mutex;
pthread_cond_t convar;
int locked = 0;
struct timeval begin;

int compareTrains(train *t1, train *t2) {
	if(t1->load_time > t2->load_time) {return 1;}
	else if(t1->load_time < t2->load_time) {return -1;}

	else if(t1->id > t2->id) {return 1;}
	else if(t1->id < t2->id) {return -1;}
	
	else {return 0;}

}

void qSort(char dir) {
	int x;
	int y;
	int start;

	//check if track is in use
	if(locked) {
		start = 1;
	} else {
		start = 0;
	}

	if(dir == 'e') {
		for(x = start; x < qlength_loweast; x++) {
			for(y = start; y < qlength_loweast - 1; y++) {
				if(compareTrains(low_eastQ[y], low_eastQ[y+1]) == 1) {
					train *temp = low_eastQ[y+1];
					low_eastQ[y+1] = low_eastQ[y];
					low_eastQ[y] = temp;
				}
			}
		}
	}
	if(dir == 'E') {
		for(x = start; x < qlength_higheast; x++) {
			for(y = start; y < qlength_higheast - 1; y++) {
				if(compareTrains(high_eastQ[y], high_eastQ[y+1]) == 1) {
					train *temp = high_eastQ[y+1];
					high_eastQ[y+1] = high_eastQ[y];
					high_eastQ[y] = temp;
				}
			}
		}
	}
	if(dir == 'w') {
		for(x = start; x < qlength_lowwest; x++) {
			for(y = start; y < qlength_lowwest - 1; y++) {
				if(compareTrains(low_westQ[y], low_westQ[y+1]) == 1) {
					train *temp = low_westQ[y+1];
					low_westQ[y+1] = low_westQ[y];
					low_westQ[y] = temp;
				}
			}
		}
	}
	if(dir == 'W') {
		for(x = start; x < qlength_highwest; x++) {
			for(y = start; y < qlength_highwest - 1; y++) {
				if(compareTrains(high_westQ[y], high_westQ[y+1]) == 1) {
					train *temp = high_westQ[y+1];
					high_westQ[y+1] = high_westQ[y];
					high_westQ[y] = temp;
				}
			}
		}
	}	
}

void qInsert(train *t) {
	if(t->direction == 'e') {
		low_eastQ[qlength_loweast] = t;
		qlength_loweast++;
	}
	if(t->direction == 'E') {
		high_eastQ[qlength_higheast] = t;
		qlength_higheast++;
	}
	if(t->direction == 'w') {
		low_westQ[qlength_lowwest] = t;
		qlength_lowwest++;
	}
	if(t->direction == 'W') {
		high_westQ[qlength_highwest] = t;
		qlength_highwest++;
	}


}

void qRemove(char dir) {
	int x = 0;
	if(dir == 'E') {
		while(x < qlength_higheast - 1) {
			high_eastQ[x] = high_eastQ[x+1];
			x++;
		}
		qlength_higheast--;
	}

	if(dir == 'W') {
		while(x < qlength_highwest - 1) {
			high_westQ[x] = high_westQ[x+1];
			x++;
		}
		qlength_highwest--;
	}
	if(dir == 'e') {
		while(x < qlength_loweast - 1) {
			low_eastQ[x] = low_eastQ[x+1];
			x++;
		}
		qlength_loweast--;
	}
	if(dir == 'w') {
		while(x < qlength_lowwest - 1) {
			low_westQ[x] = low_westQ[x+1];
			x++;
		}
		qlength_lowwest--;
	}

}

void requestTrack(train *t) {
	if(pthread_mutex_lock(&mutex) != 0) {
		fprintf(stderr, "mutex lock error");
		exit(1);
	}

	qInsert(t);
	qSort(t->direction);

	if(t->direction == 'E' || t->direction == 'W') {
		
		if(t->direction == 'E') {
			if(t->id != high_eastQ[0]->id) {
				while(high_eastQ[0]->id != t->id) {
					if(pthread_cond_wait(&convar, &mutex) != 0) {
						fprintf(stderr, "mutex wait error");
						exit(1);
					}
				}
			}
		}
		if(t->direction == 'W') {
			if(t->id != high_westQ[0]->id) {
				while(high_westQ[0]->id != t->id) {
					if(pthread_cond_wait(&convar, &mutex) != 0) {
						fprintf(stderr, "mutex wait error");
						exit(1);
					}
				}
			}
		}
		locked = 1;
		if(pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "mutex unlock error");
			exit(1);
		}
	}
	else {
		if(t->direction == 'e') {
			if(t->id != low_eastQ[0]->id) {
				while(low_eastQ[0]->id != t->id) {
					if(pthread_cond_wait(&convar, &mutex) != 0) {
						fprintf(stderr, "mutex wait error");
						exit(1);
					}
				}
			}
		}
		if(t->direction == 'w') {
			if(t->id != low_westQ[0]->id) {
				while(low_westQ[0]->id != t->id) {
					if(pthread_cond_wait(&convar, &mutex) != 0) {
						fprintf(stderr, "mutex wait error");
						exit(1);
					}
				}
			}
		}

		locked = 1;
		if(pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "mutex unlock error");
			exit(1);
		}
	}
	/*
	locked = 1;
	if(pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "mutex unlock error");
		exit(1);
	}
	*/

}

void finishTrack(train *t) {
	if(pthread_mutex_lock(&mutex) != 0) {
		fprintf(stderr, "mutex lock error");
		exit(1);
	}

	if(pthread_cond_broadcast(&convar) != 0) {
		fprintf(stderr, "broadcast error");
		exit(1);
	}

	qRemove(t->direction);
	locked = 0;

	if(pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "mutex unlock error");
		exit(1);
	}
	
}


void tokenizeTrains(char t_coll[100][100], int t_cnt) {
	int counter;
	for(counter = 1; counter <= t_cnt; counter++) {
		char delim[] = " \n";

		char *array[3];

		char *token = strtok(t_coll[counter - 1], delim);

		int i = 0;
		while(token != NULL) {
			array[i] = token;
			token = strtok(NULL, delim);
			i++;
		}

		train new_train = {counter-1, *array[0], atoi(array[1]), atoi(array[2])};

		trains[counter - 1] = new_train;

	}
}

float getTime() {
	struct timeval current;
	gettimeofday(&current, NULL);
	long begin_ms = (begin.tv_sec * 1000000) + begin.tv_usec;
	long current_ms = (current.tv_sec * 1000000) + current.tv_usec;
	return (float)(current_ms - begin_ms) / (1000000);
}

void *trainStart(void *train_item) {
	train *t = (train *)train_item;

	usleep(t->load_time * 100000);
	printf("%.2f Train %2d is ready to go %c\n", getTime(), t->id, t->direction);

	requestTrack(t);

	printf("%.2f Train %2d is ON the main track going %c\n", getTime(), t->id, t->direction);
	usleep(t->cross_time * 100000);
	printf("%.2f Train %2d is OFF the main track after going %c\n", getTime(), t->id, t->direction);

	finishTrack(t);


	pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
	if(argc < 2){
		fprintf(stderr, "need to specify a text file.\n");
		exit(1);
	}

	int size = 100;
	char train_collection[size][size];

	FILE *file = fopen(argv[1], "r");

	int pos = 0;
	while(fgets(train_collection[pos], size, file)) {
		pos++;
	}

	fclose(file);
	int train_count = pos;
	
	tokenizeTrains(train_collection, train_count);
	
	if(pthread_mutex_init(&mutex, NULL) != 0) {
		fprintf(stderr, "mutex initialization error");
		exit(1);
	}
	if(pthread_cond_init(&convar, NULL) != 0) {
		fprintf(stderr, "condition variable initialization error");
		exit(1);
	}

	pthread_attr_t attr;
	if(pthread_attr_init(&attr) != 0) {
		fprintf(stderr, "Attribute initialization error");
		exit(1);
	}
	if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0) {
		fprintf(stderr, "detachstate error");
		exit(1);
	}


	gettimeofday(&begin, NULL);


	int iter;
	//creating thread for each train
	for(iter = 0; iter < train_count; iter++) {
		if(pthread_create(&threads[iter], &attr, trainStart, &trains[iter]) != 0) {
			fprintf(stderr, "Thread creation error");
			exit(1);
		}
	}

	for(iter = 0; iter < train_count; iter++ ) {
		if(pthread_join(threads[iter], NULL) != 0) {
			fprintf(stderr, "Thread join error");
			exit(1);
		}
	}

	if(pthread_attr_destroy(&attr) != 0) {
		fprintf(stderr, "attribute destroy error");
		exit(1);
	}

	if(pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "mutex destroy error");
		exit(1);
	}

	if(pthread_cond_destroy(&convar) != 0) {
		fprintf(stderr, "convar destroy error");
		exit(1);
	}

	pthread_exit(NULL);
	
	
	
	
	return 0;
}
