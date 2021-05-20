
%{

#include <stdlib.h>
#include <string.h>

#include <tree.h>

int yylex();

extern void yyerror(char*);

%}

%union {
	struct tree tree;
	struct params params;
	
	char* text;
}

%type <tree> mainSection

%type <params> parameter
%type <params> parameters
%type <params> moreParameters

%type <text> statementHeader
%type <text> blockStatement
%type <text> statement
%type <text> output
%type <text> texts

%token <text> TEXT
%token SECTION COMMA END
%token PARAMS_BEGIN PARAMS_END
%token STATEMENT_BEGIN STATEMENT_END
%token OUTPUT_BEGIN OUTPUT_END

%start file

%%

file: metaSection SECTION mainSection
;

metaSection: /* empty */
           | PARAMS_BEGIN parameters PARAMS_END metaSection
           | STATEMENT_BEGIN metaStatement STATEMENT_END metaSection
;

parameters: /* empty */
	{
		$$ = newParams();
	}
          | parameter moreParameters
	{
		$$ = combineParams($1, $2);
	}
;

parameter: TEXT TEXT
	{
		$$ = newParams();
		addParam(&$$, $1, $2);
	}
;         

moreParameters: /* empty */
	{
		$$ = newParams();
	}
              | COMMA parameter moreParameters
	{
		$$ = combineParams($2, $3);
	}
;

metaStatement: statement
;

mainSection: /* empty */
	{
		$$ = newTree();
	}
           | mainSection TEXT
	{
		$$ = $1;
		addNode(&$$, newTextNode($2));
	}
           | mainSection statementHeader mainSection statementEnd
	{
		$$ = $1;
		addNode(&$$, newStatementNode($2, $3));
	}
           | mainSection OUTPUT_BEGIN output OUTPUT_END
	{
		$$ = $1;
		addNode(&$$, newOutputNode($3));
	}
;

statementHeader: STATEMENT_BEGIN blockStatement STATEMENT_END
	{
		$$ = $2;
	}
;

statementEnd: STATEMENT_BEGIN END STATEMENT_END
;

blockStatement: statement
	{
		$$ = $1;
	}
;

statement: TEXT texts
	{
		$$ = combineStr($1, $2);
	}
;

output: TEXT texts
	{
		$$ = combineStr($1, $2);
	}
;

texts: /* empty */
	{
		$$ = strdup("");
	}
     | TEXT texts
	{
		$$ = combineStr($1, $2);
	}
