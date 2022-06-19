#include "dll.h"
#include "sharedMemory.h"
#include "event.h"

typedef struct {
	char eventName[32];
	char data[0];
} hiMQMemory;

typedef struct {
	void* data; // hiMQMemory + 32Byte
	void* end;
	void* header;
	void* current; // read/write position
	hiEvent* ev;
} hiMQInstance;

HI_API hiMQInstance* hiMQ_createIPC(uint32_t size);

HI_API hiMQInstance* hiMQ_create(void* ptr, uint32_t size);

HI_API const char* hiMQ_getIPCName(hiMQInstance* inst);

HI_API hiMQInstance* hiMQ_openIPC(const char* name);

HI_API hiMQInstance* hiMQ_open(void* ptr, uint32_t size);

HI_API void hiMQ_close(hiMQInstance* inst);

HI_API void hiMQ_closeIPC(hiMQInstance* inst);

HI_API uint32_t hiMQ_wait(hiMQInstance* inst, uint32_t ms);

HI_API uint32_t hiMQ_get(hiMQInstance* inst);

HI_API uint32_t hiMQ_next(hiMQInstance* inst);

HI_API void hiMQ_begin(hiMQInstance* inst);

HI_API void hiMQ_end(hiMQInstance* inst, uint32_t size, uint32_t setEvent);
