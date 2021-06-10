#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "templates.h"
#include "common.h"

#define MAX_TEMPLATES (128)

typedef void (*template_t)(FILE*, va_list);
typedef size_t (*template_length_t)(va_list);

struct entry {
	const char* name;
	bool abstract;
	template_t start;
	template_t end;
	template_length_t s;
} templates[MAX_TEMPLATES];
size_t templateno = 0;

static struct entry _findTemplate(const char* name) {
	for (size_t i = 0; i < templateno; i++) {
		if (strcmp(templates[i].name, name) == 0) {
			return templates[i];
		}
	}
	
	return (struct entry) {
		.name = NULL
	};
}

void _registerTemplate(const char* name, bool abstract, template_t start, template_t end, template_length_t s) {
	if (templateno >= MAX_TEMPLATES) {
		panic("max number of templates exceeded");
	}
	
	if (_findTemplate(name).name != NULL) {
		fprintf(stderr, "warning: template name '%s' already registered; ignoring this one\n", name);
		return;
	}
	
	templates[templateno].abstract = abstract;
	templates[templateno].name = name;
	templates[templateno].start = start;
	templates[templateno].end = end;
	templates[templateno++].s = s;
}


static size_t emptyTemplateSize(va_list _) { return 0; }
static void emptyTemplate(FILE* out, va_list _) { }

template_t findTemplate(bool abstract, bool start, const char* name) {
	struct entry t = _findTemplate(name);
	if (t.name == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		return &emptyTemplate;
	}
	if (t.abstract) {
		if (!abstract) {
			fprintf(stderr, "warning: template name '%s' is abstract\n", name);	
			return &emptyTemplate;
		} else {
			if (start) {
				return t.start;
			} else {
				return t.end;
			}
		}
	} else {
		return t.start;
	}
}

static template_length_t findTemplateSize(const char* name) {
	struct entry t = _findTemplate(name);
	if (t.name == NULL) {
		fprintf(stderr, "warning: template '%s' does not exist.\n", name);
		return &emptyTemplateSize;
	}
	return t.s;
}

void _renderTemplate(const char* name, FILE* out, va_list argptr) {
	template_t t = findTemplate(false, false, name);
	t(out, argptr);
}

static void _renderTemplateStart(const char* name, FILE* out, va_list argptr) {
	template_t t = findTemplate(true, true, name);
	t(out, argptr);
}

static void _renderTemplateEnd(const char* name, FILE* out, va_list argptr) {
	template_t t = findTemplate(true, false, name);
	t(out, argptr);
}

void renderTemplate(const char* name, FILE* out, ...) {
	va_list argptr;
	va_start(argptr, out);
	
	_renderTemplate(name, out, argptr);
	
	va_end(argptr);
}

void renderTemplateStart(const char* name, FILE* out, ...) {
	va_list argptr;
	va_start(argptr, out);
	
	_renderTemplateStart(name, out, argptr);
	
	va_end(argptr);
}

void renderTemplateEnd(const char* name, FILE* out, ...) {
	va_list argptr;
	va_start(argptr, out);
	
	_renderTemplateEnd(name, out, argptr);
	
	va_end(argptr);
}

size_t _sizeTemplate(const char* name, va_list argptr) {
	template_length_t s = findTemplateSize(name);	
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
	template_t t = findTemplate(false, false, name);
	template_length_t s = findTemplateSize(name);
	
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
