#include <shared.h>

void charstr_copy(int *msg, int *buf, int length) {
	for (int i=0; i<length; i++) buf[i] = msg[i];
}