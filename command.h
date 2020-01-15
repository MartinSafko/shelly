#pragma once

#include <sys/queue.h>

#define MAX_ARGS 128

struct args_t
{
    int n;
    char* arr[MAX_ARGS];
};

struct stmt_t
{
    STAILQ_ENTRY(stmt_t) entries;
    int internal;
    int pid;
    char* name;
    struct args_t* args;
};

typedef STAILQ_HEAD(list_t, stmt_t) stmt_list_t;

struct args_t* create_args();
struct stmt_t* create_stmt(char*);
struct stmt_t* create_stmt_args(char*, struct args_t*);
struct stmt_t* create_internal_stmt(char*);

stmt_list_t* create_stmt_list();

void add_arg(struct args_t*, char*);
void add_stmt(stmt_list_t* list, struct stmt_t* stmt);

void run_command(stmt_list_t*);

void destroy_args(struct args_t*);
void destroy_stmt(struct stmt_t*);
void destroy_stmt_list(stmt_list_t*);
