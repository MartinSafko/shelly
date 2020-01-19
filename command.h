#pragma once

#include <sys/queue.h>

#define MAX_ARGS 128

struct args_t
{
    int n;
    char* arr[MAX_ARGS];
};

struct redr_t
{
    char* input;
    char* output;
    char* append;
};

struct stmt_t
{
    STAILQ_ENTRY(stmt_t) entries;
    int internal;
    int pid;
    char* name;
    struct args_t* args;
    struct redr_t* redr;
};

typedef STAILQ_HEAD(list_t, stmt_t) stmt_list_t;

struct args_t* create_args();
struct redr_t* create_redr(int, char*);
struct stmt_t* create_stmt(char*, struct redr_t*);
struct stmt_t* create_stmt_args(char*, struct args_t*, struct redr_t*);
struct stmt_t* create_internal_stmt(char*);

stmt_list_t* create_stmt_list();

void add_arg(struct args_t*, char*);
void add_stmt(stmt_list_t* list, struct stmt_t* stmt);

void combine_redr(struct redr_t*, struct redr_t*);

void destroy_args(struct args_t*);
void destroy_redr(struct redr_t*);
void destroy_stmt(struct stmt_t*);
void destroy_stmt_list(stmt_list_t*);
