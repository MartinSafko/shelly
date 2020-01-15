%{
#include "command.h"

extern void command_callback(char*, char* args[], int n);

extern void exit_callback();

extern void cd_home();
extern void cd_dash();
extern void cd_dir(const char*);

extern int yylex();
extern void yyerror(const char*);
%}

%union {
    char* str;
    struct args_t* args;
    struct stmt_t* stmt;
    struct list_t* stmt_list;
};

%token SEMICOLON NL DASH PIPE
%token EXIT CD
%token<str> STRING

%type<str> mstr;
%type<args> args
%type<stmt> stmt;
%type<stmt_list> pstm;

%%

line: expr
    | expr NL line 
    ;

expr:
    | rstm
    | rstm SEMICOLON expr
    ;

rstm: pstm          { run_command($1); destroy_stmt_list($1); }
    ;

pstm: stmt              { $$ = create_stmt_list(); add_stmt($$, $1); }
    | stmt PIPE pstm    { add_stmt($3, $1); $$ = $3; }
    ;

stmt: cdcc          { $$ = create_internal_stmt("cd"); }
    | EXIT          { $$ = create_internal_stmt("exit"); }
    | STRING        { $$ = create_stmt($1); }
    | STRING args   { $$ = create_stmt_args($1, $2); /*command_callback($1, $2.arr, $2.n);*/ }
    ;

args: STRING        { $$ = create_args(); add_arg($$, $1); }
    | args STRING   { add_arg($1, $2); $$ = $1; }
    ;

cdcc: CD            { cd_home(); }
    | CD DASH       { cd_dash(); }
    | CD STRING     { cd_dir($2); free($2); }
    | CD mstr       { /*free($2); free($3);*/ yyerror("cd: too many arguments"); }
    ;

mstr: STRING STRING
    | mstr STRING
    ;
