%option noyywrap
%option noinput
%option nounput
%option yylineno

%{
#include "parser.tab.h"
%}

%%
[ \t]
\n              { return NL; }
"-"             { return DASH; }
";"               { return SEMICOLON; }
"|"             { return PIPE; }
"#"(.)*
"exit"            { return EXIT; }
"cd"              { return CD; }
[^ ;#\t\n\|]+    { yylval.str = strdup(yytext); return STRING; }
