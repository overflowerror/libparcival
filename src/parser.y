
%token SECTION TEXT COMMA END
%token PARAMS_BEGIN PARAMS_END
%token STATEMENT_BEGIN STATEMENT_END
%token OUTPUT_BEGIN OUTPUT_END

%start file

%%

file: metaSection SECTION mainSection
;

metaSection: PARAMS_BEGIN parameters PARAMS_END
           | STATEMENT_BEGIN metaStatement STATEMENT_END
;

parameters: /* empty */
          | parameter moreParameters
;

parameter: /* empty */
         | TEXT parameter
;         

moreParameters: /* empty */
              | COMMA /* allow trailing commas */
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

statement: TEXT /* statement can't be empty */
         | TEXT statement
;

output: TEXT /* output can't be empty */
      | TEXT output
;
