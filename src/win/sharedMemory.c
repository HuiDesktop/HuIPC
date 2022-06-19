#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "sharedMemory.h"

typedef struct {
	void* ptr;
	char name[32];
	HANDLE file;
} hiSharedMemory_win;

void randStr(char* c, size_t from, size_t to) {
	static BOOL is_srand = FALSE;
	if (!is_srand) {
		srand((unsigned int)time(0));
		is_srand = TRUE;
	}
	while (from != to) {
		c[from] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[rand() % 63];
		from += 1;
	}
}

HI_API hiSharedMemory* hiSharedMemory_create(uint32_t size) {
	if (size > UINT32_MAX - 4) return NULL;
	size_t allocateSize = size;
	allocateSize += 4;
	hiSharedMemory_win* r = malloc(sizeof(hiSharedMemory_win));
	if (r == NULL) return NULL;
	ZeroMemory(r, sizeof(hiSharedMemory_win));
	randStr(r->name, 0, 31);
	r->name[31] = '\0';
	r->file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, allocateSize, r->name);
	if (r->file == NULL) goto FAILED;
	r->ptr = MapViewOfFile(r->file, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, allocateSize);
	((uint32_t*)r)[0] = size;
	return r;
FAILED:
	if (r->ptr != NULL) UnmapViewOfFile(r->ptr);
	if (r->file != NULL) CloseHandle(r->file);
	free(r);
	return NULL;
}

uint32_t hiSharedMemory_readSize(const char* name) {
	HANDLE file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(uint32_t), name);
	if (file == NULL) return 0;
	uint32_t *ptr = MapViewOfFile(file, FILE_MAP_READ, 0, 0, sizeof(uint32_t));
	if (ptr == NULL) {
		CloseHandle(file);
		return 0;
	}
	uint32_t r = ptr[0];
	UnmapViewOfFile(ptr);
	CloseHandle(file);
	return r;
}

HI_API hiSharedMemory* hiSharedMemory_open(const char* name) {
	uint32_t size = hiSharedMemory_readSize(name);
	if (size == 0) return NULL;
	size_t allocateSize = size;
	allocateSize += 4;
	hiSharedMemory_win* r = malloc(sizeof(hiSharedMemory_win));
	if (r == NULL) return NULL;
	ZeroMemory(r, sizeof(hiSharedMemory_win));
	memcpy(r->name, name, sizeof(char) * 32);
	r->file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, allocateSize, r->name);
	if (r->file == NULL) goto FAILED;
	r->ptr = MapViewOfFile(r->file, FILE_MAP_READ | FILE_MAP_WRITE, 0, 4, allocateSize - 4);
	return r;
FAILED:
	if (r->ptr != NULL) UnmapViewOfFile(r->ptr);
	if (r->file != NULL) CloseHandle(r->file);
	free(r);
	return NULL;
}

HI_API void hiSharedMemory_close(hiSharedMemory* r) {
	if (r == NULL) return;
	if (r->ptr != NULL) UnmapViewOfFile(r->ptr);
	if (((hiSharedMemory_win*)r)->file != NULL) CloseHandle(((hiSharedMemory_win*)r)->file);
	free(r);
}
