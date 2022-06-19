#include <Windows.h>
#include <string.h>
#include "sharedMemory.h"

typedef struct {
	void* ptr;
	char name[32];
	uint32_t size;
	HANDLE file;
} hiSharedMemory_win;

HI_API hiSharedMemory* hiSharedMemory_create(uint32_t size) {
	if (size > UINT32_MAX - 4) return NULL;
	size_t allocateSize = size;
	allocateSize += 4;
	hiSharedMemory_win* r = malloc(sizeof(hiSharedMemory_win));
	if (r == NULL) return NULL;
	ZeroMemory(r, sizeof(hiSharedMemory_win));
	randStr(r->name, 0, 31);
	r->name[31] = '\0';
	r->size = size;
	r->file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, allocateSize, r->name);
	if (r->file == NULL) goto FAILED;
	r->ptr = MapViewOfFile(r->file, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, allocateSize);
	if (r->ptr == NULL) {
		goto FAILED;
	}
	((uint32_t*)r->ptr)[0] = size;
	((uint32_t*)r->ptr) += 1;
	return r;
FAILED:
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
	r->size = size;
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
	if (r->ptr != NULL) UnmapViewOfFile(((uint32_t*)r->ptr) - 1);
	if (((hiSharedMemory_win*)r)->file != NULL) CloseHandle(((hiSharedMemory_win*)r)->file);
	free(r);
}
