#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include "tree.h"
#include "common.h"

#define FUNCTION_PREFIX ("template")

extern FILE* yyin;
extern int yyparse();
extern int yylineno;

struct template result;

FILE* output;
char* name;
const char* filename;

void generateHeader() {
	fprintf(output, "#include <stdio.h>\n");
	fprintf(output, "#include <stdarg.h>\n");
	fprintf(output, "\n");
	fprintf(output, "#include <templates.h>\n");
	fprintf(output, "\n");
	fprintf(output, "extern void _registerTemplate(const char*, template_t);\n");
	fprintf(output, "\n");
	for (size_t i = 0; i < result.stats.no; i++) {
		fprintf(output, "%s\n", result.stats.texts[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "static void %s_%s_(FILE* out, ...) {\n", FUNCTION_PREFIX, name);
	for (size_t i = 0; i < result.params.no; i++) {
		fprintf(output, "\t%s %s;\n", result.params.types[i], result.params.names[i]);
	}
	fprintf(output, "\t{\n");
	fprintf(output, "\t\tva_list argptr;\n");
	fprintf(output, "\t\tva_start(argptr, out);\n");
	for (size_t i = 0; i < result.params.no; i++) {
		fprintf(output, "\t\t%s = va_arg(argptr, %s);\n", result.params.names[i], result.params.types[i]);
	}
	fprintf(output, "\t\tva_end(argptr);\n");
	fprintf(output, "\t}\n");
}

char* fixText(char* text) {
	size_t textLength = strlen(text);
	size_t controlChars = 0;
	for (size_t i = 0; i < textLength; i++) {
		if (text[i] == '\t' || text[i] == '\n') {
			controlChars++;
		}
		if (text[i] == '\\') {
			controlChars++;
		}
		if (text[i] == '"') {
			controlChars++;
		}
	}
	
	char* tmp = malloc(textLength + controlChars + 1);
	if (tmp == NULL) {
		panic("malloc");
	}
	
	size_t i, j;
	
	for (i = 0, j = 0; i < textLength; i++, j++) {
		if (text[i] == '\t') {
			tmp[j++] = '\\';
			tmp[j] = 't';
		} else if (text[i] == '\n') {
			tmp[j++] = '\\';
			tmp[j] = 'n';
		} else if (text[i] == '\\' || text[i] == '"') {
			tmp[j++] = '\\';
			tmp[j] = text[i];
		} else {
			tmp[j] = text[i];
		}
	}
	
	tmp[j] = '\0';
	
	return tmp;
}

void indent(int indentation) {
	for (int i = 0; i < indentation; i++) {
		putc('\t', output);
	}
}

static void parseTree(int, struct tree);

void generateTextNode(int indentation, struct node node) {
	indent(indentation);
	char* tmp = fixText(node.value.text);
	fprintf(output, "fputs(\"%s\", out);\n", tmp);
	free(tmp);
}

void generateStatementNode(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "%s {\n", node.statement);
	
	parseTree(indentation + 1, *node.value.tree);
	
	indent(indentation);
	fprintf(output, "}\n");
}

void generateOutputNode(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "fprintf(out, %s);\n", node.value.text);
}

void parseTree(int indentation, struct tree tree) {
	for (size_t i = 0; i < tree.kidsno; i++) {
		switch(tree.kids[i].type) {
			case TEXT_NODE:
				generateTextNode(indentation, tree.kids[i]);
				break;
			case STATEMENT_NODE:
				generateStatementNode(indentation, tree.kids[i]);
				break;
			case OUTPUT_NODE:
				generateOutputNode(indentation, tree.kids[i]);
				break;
			default:
				panic("unknown node type");
		}
	}
}

void generateTree() {
	parseTree(1, result.tree);
}

void generateFooter() {
	fprintf(output, "}\n");
	fprintf(output, "__attribute__((constructor)) static void _register() {\n");
	fprintf(output, "\t_registerTemplate(\"%s\", &%s_%s_);\n", filename, FUNCTION_PREFIX, name);
	fprintf(output, "}\n");
}

void fixName() {
	size_t len = strlen(name);
	for (size_t i = 0; i < len; i++) {
		if (!((name[i] >= 'a' && name[i] <= 'z') ||
		      (name[i] >= 'A' && name[i] <= 'Z') ||
		      (name[i] >= '0' && name[i] <= '9')
		     )
		) {
			name[i] = '_';
		}
	}
}

int main(int argc, char** argv) {
	// TODO: build sane argument parsing
	if (argc != 2) {
		panic("expected input file argument");
	}

	filename = argv[1];
	
	name = alloca(strlen(filename) + 1);
	strcpy(name, filename);
	fixName();

	output = stdout;

	yyin = fopen(filename, "r");
	if (yyin == NULL) {
		panic("fopen");
	}

	if (yyparse() < 0) {
		return 1;
	}
	
	generateHeader();
	generateTree();
	generateFooter();
	
	return 0;
}

void yyerror(char* s) {
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
	exit(1);
}
