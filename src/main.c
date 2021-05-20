#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tree.h"
#include "common.h"

extern int yyparse();
extern int yylineno;

struct template result;

FILE* output;
const char* name;

void generateHeader() {
	fprintf(output, "#include <stdio.h>\n");
	fprintf(output, "#include <stdarg.h>\n");
	fprintf(output, "\n");
	for (size_t i = 0; i < result.stats.no; i++) {
		fprintf(output, "%s\n", result.stats.texts[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "void _template_%s_(FILE* out, ...) {\n", name);
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
}

int main() {
	output = stdout;
	name = "test";

	if (yyparse() < 0) {
		return 1;
	}
	
	generateHeader();
	
	generateTree();
	
	generateFooter();
}

void yyerror(char* s) {
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
	exit(1);
}
