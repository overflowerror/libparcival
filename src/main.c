
#include <stdio.h>

extern int yyparse();

int main() {
	return yyparse();
}

void yyerror(char* s) {
	extern int yylineno;
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
}
