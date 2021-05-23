#ifndef TEMPLATES_H_
#define TEMPLATES_H_

#include <stdio.h>
#include <stdlib.h>

size_t sizeTemplate(const char*, ...);
void renderTemplate(const char*, FILE*, ...);
char* renderTemplateStr(const char*, ...);

#endif
