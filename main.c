#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <getopt.h>
#include <err.h>
#include <signal.h>
#include <readline/readline.h>

#include "parser.tab.h"
#include "lexer.h"

#include "change_dir.h"

static int return_value = 0;
static int child_pid = 0;

void yyerror(const char* err)
{
    errx(2, "%s near unexpected token %s", err, yytext);
}

void exit_callback()
{
    yylex_destroy();
    exit(return_value); 
}

void command_callback(char* cmd, char* args[], int n)
{
    //puts(cmd);
    //for (int i=0; i<n; ++i)
    //    puts(args[i]);

    int pid = fork();
    if (pid == -1)
        err(1, NULL);

    char** exec_args = (char**)malloc((n+2) * sizeof(char*));
    
    exec_args[0] = cmd;
    for (int i=0; i<n; ++i)
        exec_args[i+1] = args[i];
    exec_args[n+1] = (char*)NULL;
    
    if (pid > 0)
    {
        child_pid = pid;
        
        int status;
        waitpid(pid, &status, 0);
        
        child_pid = 0;

        if (WIFEXITED(status))
            return_value = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            return_value = 128 + WTERMSIG(status);
        else
            return_value = status;
    }
    else
    {
        int ret = execvp(cmd, exec_args);
        
        // TODO: better way to handle this
        for (int i=0; i<n+1; ++i)
            free(exec_args[i]);
        free(exec_args);
        yylex_destroy();

        if (ret == -1)
            err(127, NULL);
    }

    for (int i=0; i<n+1; ++i)
        free(exec_args[i]);
    free(exec_args);
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

void handle_SIGINT(int signo)
{
    if (child_pid > 0)
    {
        kill(child_pid, signo);
        fprintf(stderr, "Killed by signal %d.\n", signo);
    }
    else
    {
        puts("");
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

void setup_handler()
{
    struct sigaction act;
    act.sa_handler = handle_SIGINT;
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
