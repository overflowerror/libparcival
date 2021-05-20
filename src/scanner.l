whitespace			[\n\t\r ]

params_begin		"{$"
params_end			"$}"

section				"%%"

statement_begin	"{%"
statement_end		"%}"

output_begin		"{{"
output_end			"}}"

block_end			"end"

type_prefixes		"enum"|"struct"
type_prefix			{type_prefixes}{whitespace}+
type_pointer		{whitespace}*"*"
type					{type_prefix}?[a-zA-Z0-9_]+{type_pointer}?

%option noyywrap
%option nodefault
%option nounput
%option noinput
%option yylineno

%x PARAMS
%x STATEMENT
%x OUTPUT
%x META_SECTION
%x MAIN_SECTION

%s META_SECTION

%%

%{
	#include <stdbool.h>
	#include <stdlib.h>
	#include <stdio.h>
	
	bool isMetaSection = true;
%}

<META_SECTION>{params_begin}		{ BEGIN(PARAMS); return PARAMS_BEGIN; }
<META_SECTION>{statement_begin}	{ BEGIN(STATEMENT); return STATEMENT_BEGIN; }
<META_SECTION>{section}				{ isMetaSection = false; BEGIN(MAIN_SECTION); }
<META_SECTION>{output_begin}		{ fprintf(stderr, "error: output block not allowed in meta section (line %d)\n", yylineno); exit(1); }
<META_SECTION>.						{ fprintf(stderr, "error: raw text not allowed in meta section (line %d)\n", yylineno); exit(1); }

<PARAMS>{type}							{ return TEXT; }
<PARAMS>{params_end}					{ BEGIN(META_SECTION); return PARAMS_END; }
<PARAMS>","								{ return COMMA; }
<PARAMS>{whitespace}+				{ /* ignore whitespaces */ }
<PARAMS>$								{ return TEXT; /* catch $ as text */ }
<PARAMS>.								{ fprintf(stderr, "error: illegal token '%s' in parameter block (line %d)\n", yytext, yylineno); exit(1); }

<STATEMENT>[^%]+						{ return TEXT; }
<STATEMENT>%							{ return TEXT; /* catch % as text */ }
<STATEMENT>{statement_end}			{ if (isMetaSection) BEGIN(META_SECTION); else BEGIN(MAIN_SECTION); return STATEMENT_END; }

<MAIN_SECTION>{params_begin}		{ fprintf(stderr, "warning: parameter block not allowed in main section (line %d); assuming to be text\n", yylineno); return TEXT; }
<MAIN_SECTION>{statement_begin}	{ BEGIN(STATEMENT); return STATEMENT_BEGIN; }
<MAIN_SECTION>{output_begin}		{ BEGIN(OUTPUT); return OUTPUT_BEGIN; }
<MAIN_SECTION>.						{ return TEXT; }

<OUTPUT>[^}]+							{ return TEXT; }
<OUTPUT>{output_end}					{ BEGIN(MAIN_SECTION); return OUTPUT_END; }
<OUTPUT>"}"								{ return TEXT; /* catch } as text */ }

