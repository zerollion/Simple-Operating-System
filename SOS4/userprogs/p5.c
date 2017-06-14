#include "../lib.h"

#define SM_KEY 		123


typedef struct {
	sem_t hello;
	sem_t world;
} SEM;

void main() {
	int i;
	SEM *b = (SEM *)smcreate(SM_KEY, sizeof(SEM));

	b->hello = screate(1);
	b->world = screate(0);

	for (i=0; i<10; i++) {
		sdown(b->hello);
		printf("Hello ");
		sup(b->world);
	}

	sleep(1000);
	sdestroy(b->hello);
	sdestroy(b->world);
	smdetach();
}
