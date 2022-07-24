#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huMessageQueue.h"

const char uint8_20[] = "HiMQ";

int main() {
	hiMQInstance* tx = hiMQ_createIPC(128);
	hiMQInstance* rx = hiMQ_openIPC(((hiSharedMemory**)(tx + 1))[0]->name);
	for (int i = 0; i < 10000000; i++) {
		hiMQ_begin(tx);
		hiMQ_ensure(tx, 20);
		strcpy(tx->current, uint8_20);
		hiMQ_end(tx, 20, 0);
		assert(hiMQ_get(rx) == 24);
		assert(strcmp(rx->current, uint8_20) == 0);
		assert(hiMQ_next(rx) == 0);
	}
	puts("circulated r/w test finished.");

	return EXIT_SUCCESS;
}