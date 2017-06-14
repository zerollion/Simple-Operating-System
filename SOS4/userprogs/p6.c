#include "../lib.h"

#define SM_KEY 		123


typedef struct {
	sem_t hello;
	sem_t world;
} SEM;

void main() {
	int i;
	SEM *b = (SEM *)smattach(SM_KEY, SM_READ_WRITE);
	
	for (i=0; i<10; i++) {
		sdown(b->world);
		printf("World! ");
		sup(b->hello);
	}

	sleep(1000);
	smdetach();
}
