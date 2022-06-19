#include "dll.h"
#include <stdint.h>

/*
*          ptr |
*              v
* | block size |     block     |
* |  uint32_t  |    <size>     |
*/

typedef struct {
	void* ptr;
	char name[32];
	uint32_t size;
} hiSharedMemory;

HI_API hiSharedMemory* hiSharedMemory_create(uint32_t size);
HI_API hiSharedMemory* hiSharedMemory_open(const char* name);
HI_API void hiSharedMemory_close(hiSharedMemory* r);
