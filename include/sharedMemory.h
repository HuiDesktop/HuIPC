#include "dll.h"
#include <stdint.h>

typedef struct {
	void* ptr;
	char name[32];
	int innerData[0];
} hiSharedMemory;

HI_API hiSharedMemory* hiSharedMemory_create(uint32_t size);
HI_API hiSharedMemory* hiSharedMemory_open(const char* name);
HI_API void hiSharedMemory_close(hiSharedMemory* r);
