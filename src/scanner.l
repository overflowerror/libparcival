whitespace			[\n\t\r ]

params_begin		"{$"
params_end			"$}"

section				"%%"

statement_begin	"{%"
statement_end		"%}"

output_begin		"{{"
output_end			"}}"

block_end_token	"end"
block_end			{whitespace}*{block_end_token}{whitespace}*

type_prefixes		"enum"|"struct"
type_prefix			{type_prefixes}{whitespace}+
type_pointer		{whitespace}*"*"
type					{type_prefix}?[a-zA-Z0-9_]+{type_pointer}?

%option noyywrap
%option nounput
%option noinput
%option yylineno

%x META_SECTION
%x MAIN_SECTION

%x PARAMS
%x STATEMENT
%x OUTPUT

%%

%{
	#include <stdbool.h>
	#include <stdlib.h>
	#include <stdio.h>
	
	#include "y.tab.h"
	
	bool isMetaSection = true;
%}

<INITIAL,META_SECTION>{params_begin}		{ BEGIN(PARAMS); return PARAMS_BEGIN; }
<INITIAL,META_SECTION>{statement_begin}	{ BEGIN(STATEMENT); return STATEMENT_BEGIN; }
<INITIAL,META_SECTION>{section}				{ isMetaSection = false; BEGIN(MAIN_SECTION); return SECTION; }
<INITIAL,META_SECTION>{output_begin}		{ fprintf(stderr, "error: output block not allowed in meta section (line %d)\n", yylineno); exit(1); }
<INITIAL,META_SECTION>{whitespace}+		{ /* ignore whitespaces */ }
<INITIAL,META_SECTION>.						{ fprintf(stderr, "error: raw text not allowed in meta section (line %d)\n", yylineno); exit(1); }

<PARAMS>{type}							{ return TEXT; }
<PARAMS>{params_end}					{ BEGIN(META_SECTION); return PARAMS_END; }
<PARAMS>","								{ return COMMA; }
<PARAMS>{whitespace}+				{ /* ignore whitespaces */ }
<PARAMS>"$"								{ return TEXT; /* catch $ as text */ }
<PARAMS>.								{ fprintf(stderr, "error: illegal token '%s' in parameter block (line %d)\n", yytext, yylineno); exit(1); }

<STATEMENT>[^%e]+						{ return TEXT; }
<STATEMENT>{block_end}				{ return END; }
<STATEMENT>"%"							{ return TEXT; /* catch % as text */ }
<STATEMENT>"e"							{ return TEXT; /* catch e as text */ }
<STATEMENT>{statement_end}			{ if (isMetaSection) BEGIN(META_SECTION); else BEGIN(MAIN_SECTION); return STATEMENT_END; }

<MAIN_SECTION>{params_begin}		{ fprintf(stderr, "warning: parameter block not allowed in main section (line %d); assuming to be text\n", yylineno); return TEXT; }
<MAIN_SECTION>{statement_begin}	{ BEGIN(STATEMENT); return STATEMENT_BEGIN; }
<MAIN_SECTION>{output_begin}		{ BEGIN(OUTPUT); return OUTPUT_BEGIN; }
<MAIN_SECTION>.						{ return TEXT; }

<OUTPUT>[^}]+							{ return TEXT; }
<OUTPUT>{output_end}					{ BEGIN(MAIN_SECTION); return OUTPUT_END; }
<OUTPUT>"}"								{ return TEXT; /* catch } as text */ }

