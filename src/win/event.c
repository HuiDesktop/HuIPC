#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "event.h"

typedef struct {
	char name[32];
	HANDLE ev;
} hiEvent_win;

void randStr(char* c, size_t from, size_t to) {
	static BOOL is_srand = FALSE;
	if (!is_srand) {
		srand((unsigned int)time(0));
		is_srand = TRUE;
	}
	while (from != to) {
		c[from] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[rand() % 63];
		from += 1;
	}
}

HI_API hiEvent* hiEvent_create() {
	hiEvent_win* r = malloc(sizeof(hiEvent_win));
	if (r == NULL) return NULL;
	ZeroMemory(r, sizeof(hiEvent_win));
	randStr(r->name, 0, 31);
	r->ev = CreateEvent(NULL, TRUE, FALSE, r->name);
	if (r->ev == NULL) {
		free(r);
		return NULL;
	}
	return (hiEvent*)r;
}

HI_API hiEvent* hiEvent_open(const char* name) {
	hiEvent_win* r = malloc(sizeof(hiEvent_win));
	if (r == NULL) return NULL;
	memcpy(r->name, name, 32);
	r->ev = OpenEvent(EVENT_ALL_ACCESS, FALSE, r->name);
	if (r->ev == NULL) {
		free(r);
		return NULL;
	}
	return (hiEvent*)r;
}

HI_API void hiEvent_close(hiEvent* r) {
	if (r == NULL) return;
	if (((hiEvent_win*)r)->ev != NULL) CloseHandle(((hiEvent_win*)r)->ev);
	free(r);
}

HI_API void hiEvent_set(hiEvent* r) {
	if (r == NULL || (((hiEvent_win*)r)->ev) == NULL) return;
	SetEvent(((hiEvent_win*)r)->ev);
}

HI_API void hiEvent_reset(hiEvent* r) {
	if (r == NULL || (((hiEvent_win*)r)->ev) == NULL) return;
	ResetEvent(((hiEvent_win*)r)->ev);
}

HI_API uint32_t hiEvent_wait(hiEvent* r, uint32_t ms) {
	if (r == NULL || (((hiEvent_win*)r)->ev) == NULL) return 0;
	return WaitForSingleObject(((hiEvent_win*)r)->ev, ms) == WAIT_OBJECT_0;
}

HI_API uint32_t hiEvent_test(hiEvent* r) {
	return hiEvent_wait(r, 0);
}
