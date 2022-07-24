#include "huMessageQueue.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NEW_OR_FAIL(type, name) type* name = malloc(sizeof(type)); if (name == NULL) goto FAILED; 

uint32_t hiMQ_create1(void* ptr, uint32_t size, hiMQInstance * inst) {
	if (size <= 32) return 1;
	hiEvent* ev = hiEvent_create();
	if (ev == NULL) return 1;
	memcpy(ptr, ev->name, 32);
	if (inst == NULL) {
		hiEvent_close(ev);
		return 1;
	}
	inst->ev = ev;
	inst->data = inst->header = inst->current = ((uint8_t*)ptr) + 32;
	inst->end = ((uint8_t*)ptr) + size;
	return 0;
}

HI_API hiMQInstance* hiMQ_create(void* ptr, uint32_t size) {
	hiMQInstance* inst = malloc(sizeof(hiMQInstance));
	if (inst == NULL) return NULL;

	if (hiMQ_create1(ptr, size, inst)) {
		free(inst);
		return NULL;
	}
	return inst;
}

HI_API hiMQInstance* hiMQ_createIPC(uint32_t size) {
	hiMQInstance* inst = malloc(sizeof(hiMQInstance) + sizeof(hiSharedMemory*));
	if (inst == NULL) return NULL;

	// create shared memory
	if (size > UINT32_MAX - 32) return NULL; // 32: to save event name
	size += 32;
	hiSharedMemory* sm = hiSharedMemory_create(size);
	if (sm == NULL) {
		free(inst);
		return NULL;
	}

	// save shared memory accessor
	((hiSharedMemory**)(inst + 1))[0] = sm;

	// create mq instance
	if (hiMQ_create1(sm->ptr, size, inst)) {
		hiSharedMemory_close(sm);
		free(inst);
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
	inst->data = inst->header = inst->current = ((uint8_t*)ptr) + 32;
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
	hiSharedMemory* s = ((hiSharedMemory**)(inst + 1))[0];
	hiMQ_close(inst);
	hiSharedMemory_close(s);
}

#define VALUE(type, base, offset) (((type*)(((uint8_t*)inst->base)+(offset)))[0])
#define CUR(type, base) VALUE(type, base, 0)

HI_API uint32_t hiMQ_wait(hiMQInstance* inst, uint32_t ms) {
	return hiEvent_wait(inst->ev, ms);
}

// here the return value will >= 4 (has value) or = 0
HI_API uint32_t hiMQ_get(hiMQInstance* inst) {
	if (CUR(uint32_t, current) == 0) {
		return 0;
	}
	if (CUR(uint32_t, current) == 1) {
		inst->current = inst->data;
		if (CUR(uint32_t, current) == 1) {
			assert(0);
			return 0;
		}
		return hiMQ_get(inst);
	}
	inst->header = inst->current;
	((uint32_t*)inst->current) += 1;
	return CUR(uint32_t, header);
}

HI_API uint32_t hiMQ_next(hiMQInstance* inst) {
	inst->current = ((uint8_t*)inst->header) + ((uint32_t*)inst->header)[0];
	return hiMQ_get(inst);
}

HI_API void hiMQ_begin(hiMQInstance* inst) {
	inst->header = inst->current;
	((uint32_t*)inst->current) += 1;
}

HI_API uint32_t hiMQ_ensure(hiMQInstance* inst, uint32_t size) {
	if (((uint8_t*)inst->current) + size <= ((uint8_t*)inst->end)) {
		return 0;
	}
	if (inst->data == inst->header || (((uint8_t*)inst->end) - ((uint8_t*)inst->data) - 4) < size) {
		return 1;
	}
	((uint32_t*)inst->data)[0] = 0;
	// where is the memory barrier?
	memmove(((uint32_t*)inst->data) + 1, inst->current, ((uint8_t*)inst->current) - ((uint8_t*)inst->header) - 4); // move the data
	((uint32_t*)inst->header)[0] = 1; // the flag referred to backing to the header
	inst->current = ((uint8_t*)inst->data) + (((uint8_t*)inst->current) - ((uint8_t*)inst->header));
	inst->header = inst->data;
	return 0;
}

// size should <= UINT32_MAX - 4, the wrote size will + 4
HI_API void hiMQ_end(hiMQInstance* inst, uint32_t size, uint32_t setEvent) {
	// UGLY
	uint32_t* commit_addr = inst->header;
	((uint8_t*)inst->current) += size;
	((uint32_t*)inst->current)[0] = 0;
	// where is the memory barrier?
	((uint32_t*)inst->header)[0] = ((uint8_t*)inst->current) - ((uint8_t*)inst->header);
	if (setEvent) {
		hiEvent_set(inst->ev);
	}
}
