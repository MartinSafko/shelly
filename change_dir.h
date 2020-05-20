#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#define PATHLEN 256

static char curdir[PATHLEN] = {0};
static char olddir[PATHLEN] = {0};

static void safe_chdir(const char* dir)
{
    int ret = chdir(dir);
    if (ret == -1)
        warn("Could not change directory to %s", dir);
}

static void safe_getcwd(char* path)
{
    char* ret = getcwd(path, PATHLEN);
    if (ret == NULL)
        err(1, "Current working dir is larger then the buffer size");
}

static inline void cd_init()
{
    safe_getcwd(curdir);
    safe_getcwd(olddir);
}

static inline void cd_update_dir()
{
    strcpy(olddir, curdir);
    safe_getcwd(curdir);
}

static inline void cd_home()
{
    char* home_dir = getenv("HOME");
    if (home_dir == NULL)
        warnx("Could not find HOME directory");

    safe_chdir(home_dir);
    cd_update_dir();
}

static inline void cd_dash()
{
    puts(olddir);
    safe_chdir(olddir);
    cd_update_dir();
}

static inline void cd_dir(const char* dir)
{
    safe_chdir(dir);
    cd_update_dir();
}

