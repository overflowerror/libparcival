#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdbool.h>

#include "tree.h"
#include "common.h"

#define PRINT_PREFIX ("print_template")
#define SIZE_PREFIX ("size_template")
#define END_SUFFIX ("end")

#define SIZE_ACCUMULATOR_VAR ("_total_size_")

extern FILE* yyin;
extern int yyparse();
extern int yylineno;

struct template result;

FILE* output;
char* name;
const char* filename;

bool isAbstract = false;

void generateHeader() {
	fprintf(output, "#include <stdio.h>\n");
	fprintf(output, "#include <stdlib.h>\n");
	fprintf(output, "#include <stdarg.h>\n");
	fprintf(output, "#include <stdbool.h>\n");
	fprintf(output, "\n");
	fprintf(output, "#include <templates.h>\n");
	fprintf(output, "\n");
	fprintf(output, "extern void _registerTemplate(const char*, bool, void (*)(FILE*, va_list), void (*)(FILE*, va_list), size_t (*)(va_list));\n");
	fprintf(output, "extern void renderTemplateStart(const char*, FILE*, ...);\n")
	fprintf(output, "extern void renderTemplateEnd(const char*, FILE*, ...);\n")
	fprintf(output, "\n");
	fprintf(output, "#define _renderTemplate(f, t, ...) renderTemplate(t, f, __VA_ARGS__)\n");
	fprintf(output, "\n");
	for (size_t i = 0; i < result.stats.no; i++) {
		fprintf(output, "%s\n", result.stats.texts[i]);
	}
	fprintf(output, "\n");
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

void generateArguments() {
	for (size_t i = 0; i < result.params.no; i++) {
		fprintf(output, "\t%s %s;\n", result.params.types[i], result.params.names[i]);
	}
	fprintf(output, "\t{\n");
	for (size_t i = 0; i < result.params.no; i++) {
		fprintf(output, "\t\t%s = va_arg(argptr, %s);\n", result.params.names[i], result.params.types[i]);
	}
	fprintf(output, "\t}\n");
}

void parseTreeSize(int indentation, struct tree tree);

void generateTextNodeSize(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "%s += %zd;\n", SIZE_ACCUMULATOR_VAR, strlen(node.value.text));
}

void generateStatementNodeSize(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "%s {\n", node.statement);
	
	parseTreeSize(indentation + 1, *node.value.tree);
	
	indent(indentation);
	fprintf(output, "}\n");
}

void generateOutputNodeSize(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "%s += snprintf(NULL, 0, %s);\n", SIZE_ACCUMULATOR_VAR, node.value.text);
}

void generateRenderNodeSize(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "%s += sizeTemplate(%s);\n", SIZE_ACCUMULATOR_VAR, node.value.text);
}

void parseTreeSize(int indentation, struct tree tree) {
	for (size_t i = 0; i < tree.kidsno; i++) {
		switch(tree.kids[i].type) {
			case TEXT_NODE:
				generateTextNodeSize(indentation, tree.kids[i]);
				break;
			case STATEMENT_NODE:
				generateStatementNodeSize(indentation, tree.kids[i]);
				break;
			case OUTPUT_NODE:
				generateOutputNodeSize(indentation, tree.kids[i]);
				break;
			case RENDER_NODE:
				generateRenderNodeSize(indentation, tree.kids[i]);
				break;
			case CHILD_NODE:
				// ignore CHILD_NODE for size calculation
				break;
			default:
				panic("unknown node type");
		}
	}
}

void generateSize() {
	fprintf(output, "static size_t %s_%s_(va_list argptr) {\n", SIZE_PREFIX, name);
	fprintf(output, "\tsize_t %s = 0;\n", SIZE_ACCUMULATOR_VAR);
	generateArguments();
	
	parseTreeSize(1, result.tree);
	
	fprintf(output, "\treturn %s;\n", SIZE_ACCUMULATOR_VAR);
	fprintf(output, "}\n");
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

void generateRenderNode(int indentation, struct node node) {
	indent(indentation);
	fprintf(output, "_renderTemplate(out, %s);\n", node.value.text);
}

void generateChildNode() {
	// assuming we are at root level in tree
	fprintf(output, "}\n");
	fprintf(output, "static void %s_%s_%s_(FILE* out, va_list argptr) {\n", PRINT_PREFIX, name, END_SUFFIX);
	generateArguments();
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
			case RENDER_NODE:
				generateRenderNode(indentation, tree.kids[i]);
				break;
			case CHILD_NODE:
				generateChildNode();
				break;
			default:
				panic("unknown node type");
		}
	}
}

void generateTree() {
	fprintf(output, "static void %s_%s_(FILE* out, va_list argptr) {\n", PRINT_PREFIX, name);
	generateArguments();
	
	parseTree(1, result.tree);
	
	fprintf(output, "}\n");
}

void generateConstructor() {
	fprintf(output, "__attribute__((constructor)) static void _register() {\n");
	if (isAbstract) {
		fprintf(output, "\t_registerTemplate(\"%s\", true, &%s_%s_, &%s_%s_%s_, &%s_%s_);\n", filename, PRINT_PREFIX, name, PRINT_PREFIX, name, END_SUFFIX, SIZE_PREFIX, name);
	} else {
		fprintf(output, "\t_registerTemplate(\"%s\", false, &%s_%s_, NULL, &%s_%s_);\n", filename, PRINT_PREFIX, name, SIZE_PREFIX, name);
	}
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

void preprocessTree(int depth, struct tree tree);

void preprocessNode(int depth, struct node node) {
	if (node.type == CHILD_NODE) {
		if (depth > 0) {
			panic("child command not allowed in statement block");
		}
		if (isAbstract) {
			panic("only one child command allowed per template");
		}
		isAbstract = true;
	} else if (node.type == STATEMENT_NODE) {
		preprocessTree(depth + 1, *node.value.tree);
	}
}

void preprocessTree(int depth, struct tree tree) {
	for (size_t i = 0; i < tree.kidsno; i++) {
		preprocessNode(depth, tree.kids[i]);
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
	
	preprocessTree(0, result.tree);
	
	generateHeader();
	generateSize();
	generateTree();
	generateConstructor();
	
	return 0;
}

void yyerror(char* s) {
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
	exit(1);
}
