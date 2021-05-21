#include <stdio.h>
#include <string.h>

#include "templates.h"
#include "common.h"

#define MAX_TEMPLATES (128)

struct {
	const char* name;
	template_t f;
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

void _registerTemplate(const char* name, template_t f) {
	if (templateno >= MAX_TEMPLATES) {
		panic("max number of templates exceeded");
	}
	
	if (_findTemplate(name) != NULL) {
		fprintf(stderr, "warning: template name '%s' already registered; ignoring this one\n", name);
		return;
	}
	
	templates[templateno].name = name;
	templates[templateno++].f = f;
}

static void emptyTemplate(FILE* out, ...) {
	fprintf(out, "Template not found.\n");
}

template_t findTemplate(const char* name) {
	template_t t = _findTemplate(name);
	
	if (t == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		return &emptyTemplate;
	}
	
	return t;
}
