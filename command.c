#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "command.h"

struct args_t* create_args()
{
    struct args_t* args = malloc(sizeof(struct args_t));
    args->n = 1;

    return args;
}

struct redr_t* create_redr(int type, char* filename)
{
    struct redr_t* redr = malloc(sizeof(struct redr_t));
    redr->input = redr->output = redr->append = NULL;

    switch (type)
    {
        case 0: redr->input  = filename; break;
        case 1: redr->output = filename; break;
        case 2: redr->append = filename; break;
    }

    return redr;
}

struct stmt_t* create_stmt(char* name, struct redr_t* redr)
{
    struct args_t* args = malloc(sizeof(struct args_t));
    args->arr[0] = name;
    args->n = 1;

    struct stmt_t* stmt = malloc(sizeof(struct stmt_t));
    stmt->internal = 0;
    stmt->name = name;
    stmt->args = args;
    stmt->redr = redr;

    return stmt;
}

struct stmt_t* create_stmt_args(char* name, struct args_t* args, struct redr_t* redr)
{
    args->arr[0] = name;

    struct stmt_t* stmt = malloc(sizeof(struct stmt_t));
    stmt->internal = 0;
    stmt->name = name;
    stmt->args = args;
    stmt->redr = redr;

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

void combine(char** old, char* new)
{
    if (new != NULL)
    {
        free(*old);
        *old = new;
    }
}

void combine_redr(struct redr_t* old, struct redr_t* new)
{
    combine(&old->input,  new->input);
    combine(&old->output, new->output);
    combine(&old->append, new->append);
    
    free(new);
}

void destroy_args(struct args_t* args)
{
    for (int i=1; i<args->n; ++i)
        free(args->arr[i]);
    free(args);
}

void destroy_redr(struct redr_t* redr)
{
    free(redr->input);
    free(redr->output);
    free(redr->append);
    
    free(redr);
}

void print_redr(struct redr_t* redr)
{
    if (redr->input != NULL)
        printf("%s ", redr->input);
    if (redr->output != NULL)
        printf("%s ", redr->output);
    if (redr->append != NULL)
        printf("%s ", redr->append);
    puts("");
}


void destroy_stmt(struct stmt_t* stmt)
{
    if (!stmt->internal)
    {
        destroy_args(stmt->args);
        if (stmt->redr != NULL)     // temporary !!!
            destroy_redr(stmt->redr);
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

    if (count == 0)
        return;

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
            if (stmt->redr != NULL)
            {
                if (stmt->redr->input != NULL)
                {
                    int fd = open(stmt->redr->input, O_RDONLY, 0644);
                    if (fd == -1)
                        err(1, "Failed to open file, %s", stmt->redr->input);
                    //close(in_out_fds[index]);
                    in_out_fds[index] = fd;
                }
                if (stmt->redr->output != NULL)
                {
                    int fd = open(stmt->redr->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1)
                        err(1, "Failed to open file, %s", stmt->redr->output);
                    //close(in_out_fds[index+1]);
                    in_out_fds[index+1] = fd;
                }
                if (stmt->redr->append != NULL)
                {
                    int fd = open(stmt->redr->append, O_WRONLY | O_APPEND | O_CREAT, 0644);
                    if (fd == -1)
                        err(1, "Failed to open file, %s", stmt->redr->append);
                    //close(in_out_fds[index+1]);
                    in_out_fds[index+1] = fd;
                }
            }

            dup2(in_out_fds[index], 0);   
            dup2(in_out_fds[index+1], 1);

            if (in_out_fds[index] != 0)
                close(in_out_fds[index]);

            if (in_out_fds[index+1] != 1)
                close(in_out_fds[index+1]);

            stmt->args->arr[stmt->args->n] = NULL;

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

    free(in_out_fds);
}
