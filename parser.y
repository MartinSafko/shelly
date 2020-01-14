%{
#include <sys/queue.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct stmt
{
    STAILQ_ENTRY(stmt) entries;
    int q;
};

STAILQ_HEAD(test, stmt);

extern void exit_callback();
extern void command_callback(char*, char* args[], int n);
extern void cd_callback();
extern void cd_dash_callback();
extern void cd_dir_callback(const char*);

extern int yylex();
extern void yyerror(const char*);
%}

%union {
    char* str;
    struct {
        int n;
        char* arr[128];
    } args;
    struct test* tailq;
};

%token SEMICOLON NL DASH PIPE
%token EXIT CD
%token<str> STRING

%type<args> args
%type<str> mstr;
%type<tailq> pstm;
%type<str> stmt;

%%

line: expr
    | expr NL line 
    ;

expr:
    | rstm
    | rstm SEMICOLON expr
    ;

rstm: pstm          { /*run_command_calback();*/ struct stmt* p; STAILQ_FOREACH(p, $1, entries) { printf("%d", p->q);}}
    ;

pstm: stmt              { puts($1); $$ = malloc(sizeof(struct test)); STAILQ_INIT($$); }
    | stmt PIPE pstm    { puts($1); struct stmt* x = (struct stmt*)malloc(sizeof(struct stmt)); x->q = 1; STAILQ_INSERT_HEAD($3, x, entries); $$ = $3; }
    ;

stmt: cdcc          { $$ = "cd"; }
    | EXIT          { $$ = "exit"; exit_callback(); }
    | STRING args   { $$ = malloc(sizeof(128)); strcpy($$, $1); command_callback($1, $2.arr, $2.n); }
    ;

args:               { $$.n = 0; }
    | args STRING   { int i = $$.n++; $$.arr[i] = $2; }
    ;

cdcc: CD            { cd_callback(); }
    | CD DASH       { cd_dash_callback(); }
    | CD STRING     { cd_dir_callback($2); free($2); }
    | CD mstr       { /*free($2); free($3);*/ yyerror("cd: too many arguments"); }
    ;

mstr: STRING STRING
    | mstr STRING
    ;
