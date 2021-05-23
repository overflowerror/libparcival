#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "templates.h"
#include "common.h"

#define MAX_TEMPLATES (128)

typedef void (*template_t)(FILE*, va_list);
typedef size_t (*template_length_t)(va_list);

struct {
	const char* name;
	template_t f;
	template_length_t s;
} templates[MAX_TEMPLATES];
size_t templateno = 0;

static template_t _findTemplate(const char* name) {
	for (size_t i = 0; i < templateno; i++) {
		if (strcmp(templates[i].name, name) == 0) {
			return templates[i].f;
		}
	}
	
	return NULL;
}

void _registerTemplate(const char* name, template_t f, template_length_t s) {
	if (templateno >= MAX_TEMPLATES) {
		panic("max number of templates exceeded");
	}
	
	if (_findTemplate(name) != NULL) {
		fprintf(stderr, "warning: template name '%s' already registered; ignoring this one\n", name);
		return;
	}
	
	templates[templateno].name = name;
	templates[templateno].f = f;
	templates[templateno++].s = s;
}

static void emptyTemplate(FILE* out, va_list _) {
	fprintf(out, "Template not found.\n");
}

void renderTemplate(const char* name, FILE* out, ...) {
	template_t t = _findTemplate(name);
	
	if (t == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		t = &emptyTemplate;
	}
	
	va_list argptr;
	va_start(argptr, out);
	
	t(out, argptr);
	
	va_end(argptr);
}
