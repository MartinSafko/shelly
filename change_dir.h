#pragma once

#include <unistd.h>

#define PATHLEN 128

static char curdir[PATHLEN] = {0};
static char olddir[PATHLEN] = {0};

void cd_init()
{
    getcwd(curdir, PATHLEN);
    getcwd(olddir, PATHLEN);
}

void cd_update_dir()
{
    strcpy(olddir, curdir);
    getcwd(curdir, PATHLEN);
}

void cd_home()
{
    chdir(getenv("HOME"));
    cd_update_dir();
}

void cd_dash()
{
    puts(olddir);
    chdir(olddir);
    cd_update_dir();
}

void cd_dir(const char* dir)
{
    chdir(dir);
    // TODO: error handling
    cd_update_dir();
}
