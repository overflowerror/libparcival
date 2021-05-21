#ifndef TEMPLATES_H_
#define TEMPLATES_H_

#include <stdio.h>

typedef void (*template_t)(FILE*, ...);

template_t findTemplate(const char*);

#endif
