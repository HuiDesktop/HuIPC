#include "huMessageQueue.h"
#include <stdlib.h>
#include <string.h>

#define NEW_OR_FAIL(type, name) type* name = malloc(sizeof(type)); if (name == NULL) goto FAILED; 

hiMQInstance* hiMQ_create1(void* ptr, uint32_t size, const char* name) {
	if (size <= 32) return NULL;
	hiEvent* ev = hiEvent_create();
	if (ev == NULL) return NULL;
	memcpy(ptr, ev->name, 32);
	hiMQInstance* inst = malloc(sizeof(hiMQInstance) + 32);
	if (inst == NULL) {
		hiEvent_close(ev);
		return NULL;
	}
	inst->ev = ev;
	inst->data = inst->header = inst->current = ((uint8_t*)ptr) + 32;
	inst->end = ((uint8_t*)ptr) + size;
	memcpy(((uint8_t*)inst) + sizeof(hiMQInstance), name, 32);
	return inst;
}

HI_API hiMQInstance* hiMQ_create(void* ptr, uint32_t size) {
	if (size <= 32) return NULL;
	hiEvent* ev = hiEvent_create();
	if (ev == NULL) return NULL;
	memcpy(ptr, ev->name, 32);
	hiMQInstance* inst = malloc(sizeof(hiMQInstance));
	if (inst == NULL) {
		hiEvent_close(ev);
		return NULL;
	}
	inst->ev = ev;
	inst->data = inst->header = inst->current = ((uint8_t*)ptr) + 32;
	inst->end = ((uint8_t*)ptr) + size;
	return inst;
}

HI_API hiMQInstance* hiMQ_createIPC(uint32_t size) {
	if (size > UINT32_MAX - sizeof(hiMQMemory)) return NULL;
	size += sizeof(hiMQMemory);
	hiSharedMemory* sm = hiSharedMemory_create(size);
	if (sm == NULL) return NULL;
	hiMQInstance* inst = hiMQ_create1(sm->ptr, size, sm->name);
	if (inst == NULL) {
		hiSharedMemory_close(sm);
		return NULL;
	}
	return inst;
}

HI_API const char* hiMQ_getIPCName(hiMQInstance* inst) {
	return (char*)(((uint8_t*)inst->data) + sizeof(hiMQInstance));
}

HI_API hiMQInstance* hiMQ_openIPC(const char* name) {
	hiSharedMemory* sm = hiSharedMemory_open(name);
	if (sm == NULL) return NULL;
	hiMQInstance* inst = hiMQ_open(sm->ptr, sm->size);
	if (inst == NULL) {
		hiSharedMemory_close(sm);
		return NULL;
	}
	return inst;
}

HI_API hiMQInstance* hiMQ_open(void* ptr, uint32_t size) {
	if (size <= 32) return NULL;
	// 1. Open event
	hiEvent* ev = hiEvent_open(((hiMQMemory*)ptr)->eventName);
	if (ev == NULL) {
		return NULL;
	}
	// 2. New instance
	hiMQInstance* inst = malloc(sizeof(hiMQInstance));
	if (inst == NULL) {
		hiEvent_close(ev);
		return NULL;
	}
	inst->ev = ev;
	inst->data = inst->current = ((uint8_t*)ptr) + 32;
	inst->end = ((uint8_t*)ptr) + size;
	return inst;
}

HI_API void hiMQ_close(hiMQInstance* inst) {
	if (inst == NULL) return;
	if (inst->ev) hiEvent_close(inst->ev);
	free(inst);
}

HI_API void hiMQ_closeIPC(hiMQInstance* inst) {
	if (inst == NULL) return;
	hiSharedMemory_close(((uint8_t*)inst->data) - 32);
	hiMQ_close(inst);
}

#define VALUE(type, offset) (((type*)(((uint8_t*)inst->current)+(offset)))[0])
#define CUR(type) VALUE(type, 0)

HI_API uint32_t hiMQ_wait(hiMQInstance* inst, uint32_t ms) {
	return hiEvent_wait(inst->ev, ms);
}

HI_API uint32_t hiMQ_get(hiMQInstance* inst) {
	if (CUR(uint32_t) == 0) {
		return 0;
	}
	inst->header = inst->current;
	((uint32_t*)inst->current) += 1;
	return CUR(uint32_t);
}

HI_API uint32_t hiMQ_next(hiMQInstance* inst) {
	inst->current = ((uint8_t*)inst->header) + ((uint32_t*)inst->header)[0];
	return hiMQ_get(inst);
}

HI_API void hiMQ_begin(hiMQInstance* inst) {
	inst->header = inst->current;
	((uint32_t*)inst->current) += 1;
}

// size should <= UINT32_MAX - 4
HI_API void hiMQ_end(hiMQInstance* inst, uint32_t size, uint32_t setEvent) {
	size += 4;
	uint32_t* commit_addr = inst->header;
	((uint8_t*)inst->header) += size;
	((uint32_t*)inst->header)[0] = 0;
	// where is the memory barrier?
	commit_addr[0] = size;
	if (setEvent) {
		hiEvent_set(inst->ev);
	}
}
