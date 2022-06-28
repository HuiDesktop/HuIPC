#include "tools.h"
#include <time.h>
#include <stdlib.h>

void randStr(char* c, size_t from, size_t to) {
	static uint32_t is_srand = 0;
	if (!is_srand) {
		srand((unsigned int)time(0));
		is_srand = 1;
	}
	while (from != to) {
		c[from] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[rand() % 62];
		from += 1;
	}
	c[to] = '\0';
}
