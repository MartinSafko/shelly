#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "command.h"

struct args_t* create_args()
{
    struct args_t* args = malloc(sizeof(struct args_t));
    args->n = 1;

    return args;
}

struct stmt_t* create_stmt(char* name)
{
    struct args_t* args = malloc(sizeof(struct args_t));
    args->arr[0] = name;
    args->n = 1;

    struct stmt_t* stmt = malloc(sizeof(struct stmt_t));
    stmt->internal = 0;
    stmt->name = name;
    stmt->args = args;

    return stmt;
}

struct stmt_t* create_stmt_args(char* name, struct args_t* args)
{
    args->arr[0] = name;

    struct stmt_t* stmt = malloc(sizeof(struct stmt_t));
    stmt->internal = 0;
    stmt->name = name;
    stmt->args = args;

    return stmt;
}

struct stmt_t* create_internal_stmt(char* name)
{
    struct stmt_t* stmt = malloc(sizeof(struct stmt_t));
    stmt->internal = 1;
    stmt->name = name;

    return stmt;
}

stmt_list_t* create_stmt_list()
{
    stmt_list_t* list = malloc(sizeof(stmt_list_t));
    STAILQ_INIT(list);

    return list;
}

void add_arg(struct args_t* args, char* arg)
{
    args->arr[args->n++] = arg;
}

void add_stmt(stmt_list_t* list, struct stmt_t* stmt)
{
    STAILQ_INSERT_HEAD(list, stmt, entries);
}

void destroy_args(struct args_t* args)
{
    for (int i=1; i<args->n; ++i)
        free(args->arr[i]);
    free(args);
}

void destroy_stmt(struct stmt_t* stmt)
{
    if (!stmt->internal)
    {
        destroy_args(stmt->args);
        free(stmt->name);
    }

    free(stmt);
}

void destroy_stmt_list(stmt_list_t* list)
{
    while (!STAILQ_EMPTY(list))
    {
        struct stmt_t* stmt = STAILQ_FIRST(list);
        STAILQ_REMOVE_HEAD(list, entries);
        destroy_stmt(stmt);
    }
    free(list);
}

void run_command(stmt_list_t* list)
{
    int count = 0;

    struct stmt_t* stmt;
    STAILQ_FOREACH(stmt, list, entries)
        if (!stmt->internal)
            ++count;

    const int fds_count = (count-1)*2 + 2;

    int* in_out_fds = malloc(fds_count * sizeof(int));
    in_out_fds[0] = 0;
    in_out_fds[fds_count - 1] = 1;

    for (int i=1; i<fds_count-2; i += 2)
    {
        int pd[2];
        pipe(pd);
        in_out_fds[i] = pd[1];
        in_out_fds[i+1] = pd[0]; 
    }

    int index = 0;
    STAILQ_FOREACH(stmt, list, entries)
    {
        if (stmt->internal)
            continue;

        int pid = fork();
        if (pid == -1)
            err(1, NULL);

        if (pid == 0)
        {   
            dup2(in_out_fds[index], 0);   
            dup2(in_out_fds[index+1], 1);

            if (in_out_fds[index] != 0)
                close(in_out_fds[index]);

            if (in_out_fds[index+1] != 1)
                close(in_out_fds[index+1]);

            // just to test th arr argument
            //char** args = malloc(2*sizeof(char*));
            //args[0] = stmt->name;
            //args[1] = NULL;
            //int ret = execvp(stmt->name, args);

            int ret = execvp(stmt->name, stmt->args->arr);
            if (ret == -1)
                err(127, NULL);
        }

        if (in_out_fds[index] != 0)
            close(in_out_fds[index]);
        if (in_out_fds[index+1] != 1)
            close(in_out_fds[index+1]);
            
        stmt->pid = pid;

        index += 2;
    }

    STAILQ_FOREACH(stmt, list, entries)
    {
        int status;
        waitpid(stmt->pid, &status, 0);
    }
}
