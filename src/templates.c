#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "templates.h"
#include "common.h"

#define MAX_TEMPLATES (128)

typedef void (*template_t)(FILE*, va_list);
typedef size_t (*template_length_t)(va_list);

struct entry {
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

static template_length_t _findTemplateSize(const char* name) {
	for (size_t i = 0; i < templateno; i++) {
		if (strcmp(templates[i].name, name) == 0) {
			return templates[i].s;
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


static size_t emptyTemplateSize(va_list _) { return 0; }
static void emptyTemplate(FILE* out, va_list _) { }

void _renderTemplate(const char* name, FILE* out, va_list argptr) {
	template_t t = _findTemplate(name);
	
	if (t == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		t = &emptyTemplate;
	}
	
	t(out, argptr);
}

void renderTemplate(const char* name, FILE* out, ...) {
	va_list argptr;
	va_start(argptr, out);
	
	_renderTemplate(name, out, argptr);
	
	va_end(argptr);
}

size_t _sizeTemplate(const char* name, va_list argptr) {
	template_length_t s = _findTemplateSize(name);
	
	if (s == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		s = &emptyTemplateSize;
	}
	
	return s(argptr);
}

size_t sizeTemplate(const char* name, ...) {
	size_t result;

	va_list argptr;
	va_start(argptr, name);
	
	result = _sizeTemplate(name, argptr);
	
	va_end(argptr);
	
	return result;
}

char* renderTemplateStr(const char* name, ...) {
	template_t t = _findTemplate(name);
	template_length_t s = _findTemplateSize(name);
	
	if (t == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		t = &emptyTemplate;
		s = &emptyTemplateSize;
	}
	
	size_t length;
	char* result;
	
	va_list argptr, argptr2;
	va_start(argptr, name);
	va_copy(argptr2, argptr);
	
	length = s(argptr);
	
	va_end(argptr);
	
	result = malloc(length + 1);
	if (result == NULL) {
		va_end(argptr2);
		return NULL;
	}
	
	FILE* out = fmemopen(result, length + 1, "w");
	if (out == NULL) {
		va_end(argptr2);
		return NULL;
	}
	
	t(out, argptr2);
	fclose(out);
	
	va_end(argptr2);
	
	result[length] = '\0';
	
	return result;
}
