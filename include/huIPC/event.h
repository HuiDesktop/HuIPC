#include "dll.h"
#include "tools.h"

typedef struct {
	char name[32];
} hiEvent;

HI_API hiEvent* hiEvent_create();

HI_API hiEvent* hiEvent_open(const char* name);

HI_API void hiEvent_close(hiEvent* r);

HI_API void hiEvent_set(hiEvent* r);

HI_API void hiEvent_reset(hiEvent* r);

HI_API uint32_t hiEvent_wait(hiEvent* r, uint32_t ms);

HI_API uint32_t hiEvent_test(hiEvent* r);
