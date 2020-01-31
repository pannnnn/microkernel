#include <shared.h>
#include <lib_periph_bwio.h>

void charstr_copy(char *msg, char *buf, int length) {
	for (int i=0; i < length; i++) {
		buf[i] = msg[i];
	}
}