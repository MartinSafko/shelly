%union {
    char* str;
    struct {
        int n;
        char* arr[128];
    } args;
}

%token SEMICOLON NL DASH
%token EXIT CD
%token<str> STRING

%type<args> args

%{
extern void exit_callback();
extern void command_callback(char*, char* args[], int n);
extern void cd_callback();
extern void cd_dash_callback();
extern void cd_dir_callback(const char*);

extern int yylex();
extern void yyerror(const char*);
%}

%%

line: expr
    | expr NL line 
    ;

expr:
    | stmt
    | stmt SEMICOLON expr
    ;

stmt: cdcc
    | EXIT          { exit_callback(); }
    | STRING args   { command_callback($1, $2.arr, $2.n); }
    ;

args:               { $$.n = 0; }
    | args STRING   { int i = $$.n++; $$.arr[i] = $2; }
    ;

cdcc: CD            { cd_callback(); }
    | CD DASH       { cd_dash_callback(); }
    | CD STRING     { cd_dir_callback($2); free($2); }
    | CD STRING STRING { yyerror("too many arguments"); }
