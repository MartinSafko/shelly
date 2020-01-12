%option noyywrap
%option noinput
%option nounput

%{
#include "parser.tab.h"
%}

%%
[ \t]
\n              { return NL; }
"-"             { return DASH; }
";"               { return SEMICOLON; }
"#"(.)*
"exit"            { return EXIT; }
"cd"              { return CD; }
[^ ;#\t\n]+    { yylval.str = strdup(yytext); return STRING; }
