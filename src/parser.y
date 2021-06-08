
%{

#include <stdlib.h>
#include <string.h>

#include <tree.h>

int yylex();

extern void yyerror(char*);

extern struct template result;

%}

%union {
	struct tree tree;
	struct params params;
	struct template template;
	
	char* text;
	int nodeType;
}

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
%type <text> text

%type <nodeType> structureType

%token <text> TEXT
%token <text> WHITESPACE
%token SECTION COMMA END
%token PARAMS_BEGIN PARAMS_END
%token STATEMENT_BEGIN STATEMENT_END
%token STRUCTURE_BEGIN STRUCTURE_END
%token OUTPUT_BEGIN OUTPUT_END
%token RENDER EXTENDS CHILD OPEN_PARENTHESES CLOSE_PARENTHESES

%start template

%%

template: metaSection SECTION mainSection
	{
		result = $1;
		result.tree = $3;
	}
/*        | mainSection
	{
		$$ = newTemplate();
		$$.tree = $1;
	}
*/
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
           | STRUCTURE_BEGIN optionalWhitespaces structureType optionalWhitespaces OPEN_PARENTHESES text CLOSE_PARENTHESES optionalWhitespaces metaSection
   {
   	$$ = $9;
   	switch($3) {
   		case RENDER_NODE:
		   	yyerror("render command not allowed in meta section; ignoring");
		   	break;
		   case CHILD_NODE:
		   	yyerror("child command not allowed in meta section; ignoring");
		   	break;
		   case EXTENDS_TOKEN:
		   	if ($$.parent != NULL) {
		   		yyerror("template can only extent one parent");
		   		YYERROR;
		   	}
		   	$$.parent = $6;
		   	break;
		   default:
		   	yyerror("unhandled structure block command (internal error)");
		   	YYERROR;
		}
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
           | mainSection text
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
           | mainSection STRUCTURE_BEGIN optionalWhitespaces structureType optionalWhitespaces OPEN_PARENTHESES text CLOSE_PARENTHESES optionalWhitespaces STRUCTURE_END
   {	
   	$$ = $1;
   	switch($4) {
   		case RENDER_NODE:
		   	addNode(&$$, newRenderNode($7));
		   	break;
		   case CHILD_NODE:
		   	addNode(&$$, newChildNode());
		   	break;
		   case EXTENDS_TOKEN:
		   	yyerror("extends command not allowed in main section; ignoring");
		   	break;
		   default:
		   	yyerror("unhandled structure block command (internal error)");
		   	YYERROR;
		}
   }
;

optionalWhitespaces: /* empty */
                  | WHITESPACE optionalWhitespaces
;

structureType: RENDER
	{
		$$ = RENDER_NODE;
	}
             | EXTENDS
	{
		$$ = EXTENDS_TOKEN;
	}
             | CHILD
	{
		$$ = CHILD_NODE;
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

text: TEXT
	{
		$$ = $1;
	}
	 | WHITESPACE
	{
		$$ = $1;
	}
    | TEXT text
	{
		$$ = combineStr($1, $2);
	}
	 | WHITESPACE text
	{
		$$ = combineStr($1, $2);
	}

texts: /* empty */
	{
		$$ = strdup("");
	}
     | text
	{
		$$ = $1;
	}
