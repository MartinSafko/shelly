#include <stdio.h>
#include <getopt.h>
#include <err.h>
#include <signal.h>
#include <readline/readline.h>

#include "parser.tab.h"
#include "lexer.h"

#include "change_dir.h"
#include "execute.h"

void yyerror(const char* err)
{
    fprintf(stderr, "line %d: %s\n", yylineno, err);
    return_value = 2;
}

void exit_callback()
{
    yylex_destroy();
    exit(return_value); 
}

int command_mode(const char* command)
{
    yy_scan_string(command);
    yyparse();
    yylex_destroy();

    return return_value;
}

int shell_file_mode(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        err(1, "%s", filename);

    yyin = fp;

    yyparse();
    yylex_destroy();

    fclose(fp);

    return return_value;
}

void handle_sigint(int signo)
{
    if (child_pid > 0)
    {
        kill(child_pid, signo);
    }
    else
    {
        rl_crlf();
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

void setup_handler()
{
    struct sigaction act;
    act.sa_handler = handle_sigint;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(SIGINT, &act, NULL);
}

int interactive_mode()
{
    setup_handler();

    char* buf;
    while ((buf = readline(">> ")) != NULL)
    {
        yy_scan_string(buf);
        yyparse();
        yylex_destroy();
        free(buf);
    }
    
    puts("exit");
    return return_value;
}

int main(int argc, char** argv)
{
    cd_init();

    int opt;
    while ((opt = getopt(argc, argv, "c:")) != -1)
    {
        switch (opt)
        {
            case 'c':
                return command_mode(optarg);
            default:
                errx(1, "Usage: %s [-c commands] [shell file]", argv[0]);
        }
    }
    
    if (argc > 2)
        errx(1, "Usage: %s [-c commands] [shell file]", argv[0]);

    if (argc == 2)  // we have a shell file
        return shell_file_mode(argv[1]);

    return interactive_mode();
}
