#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "common.h"


void _panic(const char* function, const char* format, ...) {
	fprintf(stderr, "panic: %s: ", function);

	va_list argptr;
	va_start(argptr, format);
	
	vfprintf(stderr, format, argptr);
	
	va_end(argptr);
	
	if (errno != 0) {
		fprintf(stderr, ": %s", strerror(errno));
	}
	
	fprintf(stderr, "\n");
	
	exit(4);
}
