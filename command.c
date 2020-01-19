#include <stdlib.h>

#include <stdio.h>
#include "command.h"
#include "utils.h"

struct args_t* create_args()
{
    struct args_t* args = safe_malloc(sizeof(struct args_t));
    args->n = 1;

    return args;
}

struct redr_t* create_redr(int type, char* filename)
{
    struct redr_t* redr = safe_malloc(sizeof(struct redr_t));
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
    struct args_t* args = safe_malloc(sizeof(struct args_t));
    args->arr[0] = name;
    args->n = 1;

    struct stmt_t* stmt = safe_malloc(sizeof(struct stmt_t));
    stmt->internal = 0;
    stmt->name = name;
    stmt->args = args;
    stmt->redr = redr;

    return stmt;
}

struct stmt_t* create_stmt_args(char* name, struct args_t* args, struct redr_t* redr)
{
    args->arr[0] = name;

    struct stmt_t* stmt = safe_malloc(sizeof(struct stmt_t));
    stmt->internal = 0;
    stmt->name = name;
    stmt->args = args;
    stmt->redr = redr;

    return stmt;
}

struct stmt_t* create_internal_stmt(char* name)
{
    struct stmt_t* stmt = safe_malloc(sizeof(struct stmt_t));
    stmt->internal = 1;
    stmt->name = name;

    return stmt;
}

stmt_list_t* create_stmt_list()
{
    stmt_list_t* list = safe_malloc(sizeof(stmt_list_t));
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

