%option noyywrap
%option noinput
%option nounput
%option yylineno

%{
#include "parser.tab.h"
%}

%%
[ \t]           // ignore whitespace
"#"(.)*         // ignore comments
\n              { return NL; }
"-"             { return DASH; }
";"             { return SEMICOLON; }
"|"             { return PIPE; }
"<"             { return IN; }
">"             { return OUT; }
">>"            { return APP; }
"cd"            { return CD; }
"exit"          { return EXIT; }
[^ ;#\t\n\|><]+   { yylval.str = strdup(yytext); return STRING; }
