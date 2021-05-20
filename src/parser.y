
%{

int yylex();

extern void yyerror(char*);

%}

%token SECTION TEXT COMMA END
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
          | parameter moreParameters
;

parameter: TEXT texts
;         

moreParameters: /* empty */
              | COMMA parameter moreParameters
;

metaStatement: statement
;

mainSection: /* empty */
           | TEXT mainSection
           | STATEMENT_BEGIN blockStatement STATEMENT_END mainSection
           | OUTPUT_BEGIN output OUTPUT_END mainSection
;

blockStatement: statement
;

statement: TEXT texts
;

output: TEXT texts
;

texts: /* empty */
     | TEXT texts
