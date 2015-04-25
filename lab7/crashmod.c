#include "crashmod.h"

#include <stdio.h>
#include <signal.h>

int crash_counter[FS_COUNT]; //global, init to zero

void set_counter(int i, int nc) {
	crash_counter[i] = nc;
}

void set_counter_rel(int i, int nc) {
	crash_counter[i] += nc;
}

int dec_counter_(int i) {
	if (crash_counter[i] > 0) {
		--crash_counter[i];
		if (!crash_counter[i]) {
			printf("Crashing! (%d)\n", i);
			raise(SIGSEGV);
		}
	}
	return crash_counter[i];
}

int dec_counter(int i) {
	int a;
	a = dec_counter_(i);
	if (i > 0) {
		dec_counter_(0);
	}
	return a;
}

int print_counter(int i) {
	printf("crash_counter[%d] = %d\n", i, crash_counter[i]);
}
