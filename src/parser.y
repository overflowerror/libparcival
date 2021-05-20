
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
	struct template template;
	
	char* text;
}

%type <template> template
%type <template> metaSection

%type <tree> mainSection

%type <params> parameter
%type <params> parameters
%type <params> moreParameters

%type <text> statementHeader
%type <text> blockStatement
%type <text> metaStatement
%type <text> statement
%type <text> output
%type <text> texts

%token <text> TEXT
%token SECTION COMMA END
%token PARAMS_BEGIN PARAMS_END
%token STATEMENT_BEGIN STATEMENT_END
%token OUTPUT_BEGIN OUTPUT_END

%start template

%%

template: metaSection SECTION mainSection
	{
		$$ = $1;
		$$.tree = $3;
	}
        | mainSection
	{
		$$ = newTemplate();
		$$.tree = $1;
	}
;

metaSection: /* empty */
	{
		$$ = newTemplate();
	}
           | PARAMS_BEGIN parameters PARAMS_END metaSection
	{
		$$ = $4;
		if ($$.params.no != 0) {
			yyerror("only one parameter block allowed in meta section");
			YYERROR;
		}
		$$.params = $2;
	}
           | STATEMENT_BEGIN metaStatement STATEMENT_END metaSection
	{
		$$ = $4;
		addStat(&$$.stats, $2);
	}
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
	{
		$$ = $1;
	}
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
