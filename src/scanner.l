whitespace			[\n\t\r ]

params_begin		"{$"
params_end			"$}"

section				"%%"[\t ]*\n

statement_begin	[ \t]*"{%"
statement_end		"%}"[ \t]*"\n"?

output_begin		"{{"
output_end			"}}"

structure_begin	"{#"
structure_end		"#}"

block_end_token	"end"|"END"
block_end			{whitespace}*{block_end_token}{whitespace}*

render_token		"render"|"RENDER"

id						[a-zA-Z_][a-zA-Z0-9_]*

type_prefixes		"enum"|"struct"
type_prefix			{type_prefixes}{whitespace}+
type_pointer		{whitespace}*"*"
type_or_id			{type_prefix}?{id}{type_pointer}*

%option noyywrap
%option nounput
%option noinput
%option yylineno

%x META_SECTION
%x MAIN_SECTION

%x PARAMS
%x STATEMENT
%x STRUCTURE
%x OUTPUT

%{
	#include <stdbool.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	
	#include <tree.h>
	
	#include "y.tab.h"
	
	bool isMetaSection = true;
%}

%%

<INITIAL,META_SECTION>{params_begin}		{ BEGIN(PARAMS); return PARAMS_BEGIN; }
<INITIAL,META_SECTION>{statement_begin}	{ BEGIN(STATEMENT); return STATEMENT_BEGIN; }
<INITIAL,META_SECTION>{section}				{ isMetaSection = false; BEGIN(MAIN_SECTION); return SECTION; }
<INITIAL,META_SECTION>{structure_begin}	{ BEGIN(STRUCTURE); return STRUCTURE_BEGIN; }
<INITIAL,META_SECTION>{output_begin}		{ fprintf(stderr, "error: output block not allowed in meta section (line %d)\n", yylineno); exit(1); }
<INITIAL,META_SECTION>{whitespace}+			{ /* ignore whitespaces */ }
<INITIAL,META_SECTION>.							{ fprintf(stderr, "error: raw text not allowed in meta section (line %d)\n", yylineno); exit(1); }

<PARAMS>{type_or_id}					{ yylval.text = strdup(yytext); return TEXT; }
<PARAMS>{params_end}					{ BEGIN(META_SECTION); return PARAMS_END; }
<PARAMS>","								{ return COMMA; }
<PARAMS>{whitespace}+				{ /* ignore whitespaces */ }
<PARAMS>.								{ fprintf(stderr, "error: illegal token '%s' in parameter block (line %d)\n", yytext, yylineno); exit(1); }

<STATEMENT>[^%e]+						{ yylval.text = strdup(yytext); return TEXT; }
<STATEMENT>{block_end}				{ return END; }
<STATEMENT>"%"							{ yylval.text = strdup(yytext); return TEXT; /* catch % as text */ }
<STATEMENT>"e"							{ yylval.text = strdup(yytext); return TEXT; /* catch e as text */ }
<STATEMENT>{statement_end}			{ BEGIN(isMetaSection ? META_SECTION : MAIN_SECTION); return STATEMENT_END; }

<STRUCTURE>[^#r() \t\n]+			{ yylval.text = strdup(yytext); return TEXT; }
<STRUCTURE>{whitespace}+			{ yylval.text = strdup(yytext); return WHITESPACE; }
<STRUCTURE>{render_token}			{ return RENDER; }
<STRUCTURE>"("							{ return OPEN_PARENTHESES; }
<STRUCTURE>")"							{ return CLOSE_PARENTHESES; }
<STRUCTURE>"r"							{ yylval.text = strdup(yytext); return TEXT; /* catch r as text */ }
<STRUCTURE>"#"							{ yylval.text = strdup(yytext); return TEXT; /* catch # as text */ }
<STRUCTURE>{structure_end}			{ BEGIN(isMetaSection ? META_SECTION : MAIN_SECTION); return STRUCTURE_END; }

<MAIN_SECTION>{params_begin}		{ fprintf(stderr, "warning: parameter block not allowed in main section (line %d); assuming to be text\n", yylineno); yylval.text = strdup(yytext); return TEXT; }
<MAIN_SECTION>{statement_begin}	{ BEGIN(STATEMENT); return STATEMENT_BEGIN; }
<MAIN_SECTION>{structure_begin}	{ BEGIN(STRUCTURE); return STRUCTURE_BEGIN; }
<MAIN_SECTION>{output_begin}		{ BEGIN(OUTPUT); return OUTPUT_BEGIN; }
<MAIN_SECTION>\n						{ yylval.text = strdup(yytext); return TEXT; }
<MAIN_SECTION>.						{ yylval.text = strdup(yytext); return TEXT; }

<OUTPUT>[^}]+							{ yylval.text = strdup(yytext); return TEXT; }
<OUTPUT>{output_end}					{ BEGIN(MAIN_SECTION); return OUTPUT_END; }
<OUTPUT>"}"								{ yylval.text = strdup(yytext); return TEXT; /* catch } as text */ }

