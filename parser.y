%{
#include "command.h"
#include "execute.h"
#include "change_dir.h"

extern void exit_callback();
extern int yylex();
extern void yyerror(const char*);
%}

%union {
    char* str;
    struct args_t* args;
    struct redr_t* redr;
    struct stmt_t* stmt;
    struct list_t* stmt_list;
};

%token SEMICOLON NL DASH PIPE IN OUT APP
%token EXIT CD
%token<str> STRING

%type<str> mstr;
%type<args> args
%type<redr> redr reds
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
    | EXIT          { $$ = create_internal_stmt("exit"); exit_callback(); }
    | STRING        { $$ = create_stmt($1, 0); }
    | STRING args   { $$ = create_stmt_args($1, $2, 0); }

// Redirection hell
    | STRING reds                   { $$ = create_stmt($1, $2); }
    | reds STRING                   { $$ = create_stmt($2, $1); }
    | reds STRING reds              { combine_redr($1, $3); $$ = create_stmt($2, $1); }
    | STRING args reds              { $$ = create_stmt_args($1, $2, $3); }
    | STRING reds args              { $$ = create_stmt_args($1, $3, $2); }
    | STRING reds args reds         { combine_redr($2, $4); $$ = create_stmt_args($1, $3, $2); }
    | reds STRING reds args reds    { combine_redr($1, $3); combine_redr($1, $5); $$ = create_stmt_args($2, $4, $1); }
    ;

args: STRING        { $$ = create_args(); add_arg($$, $1); }
    | args STRING   { add_arg($1, $2); $$ = $1; }
    ;

reds: redr          { $$ = $1; }
    | reds redr     { combine_redr($1, $2);  $$ = $1; }
    ;

redr: IN STRING     { $$ = create_redr(0, $2); }       
    | OUT STRING    { $$ = create_redr(1, $2); }
    | APP STRING    { $$ = create_redr(2, $2); } 

cdcc: CD            { cd_home(); }
    | CD DASH       { cd_dash(); }
    | CD STRING     { cd_dir($2); free($2); }
    | CD mstr       { yyerror("cd: too many arguments"); }
    ;

mstr: STRING STRING { free($1); free($2); }
    | mstr STRING   { free($2); }
    ;
