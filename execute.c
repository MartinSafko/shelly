#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "execute.h"
#include "utils.h"

int return_value = 0;
int child_pid = 0;

void setup_redirect_fds(struct stmt_t* stmt, int* input, int* output)
{
    if (stmt->redr == NULL)
        return;
            
    if (stmt->redr->input != NULL)
    {
        close(*input);
        *input = safe_open(stmt->redr->input, O_RDONLY, 0644);
    }
    if (stmt->redr->output != NULL)
    {
        close(*output);
        *output = safe_open(stmt->redr->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (stmt->redr->append != NULL)
    {
        close(*output);
        *output = safe_open(stmt->redr->append, O_WRONLY | O_APPEND | O_CREAT, 0644);
    }
}

void close_extra_fds(int* inputs, int* output)
{
    if (*inputs != 0) close(*inputs);
    if (*output != 1) close(*output);
}

void run_command_with_premade_pipes(stmt_list_t* list)
{
    int count = 0;

    struct stmt_t* stmt;
    STAILQ_FOREACH(stmt, list, entries)
        if (!stmt->internal)
            ++count;

    if (count == 0)
        return;

    const int fds_count = (count-1)*2 + 2;
    
    // input_fd cmd output_fd:
    // fd[0] cmd1 fd[1] | fd[2] cmd2 fd[3] ...
    int* in_out_fds = safe_malloc(fds_count * sizeof(int));
    in_out_fds[0] = 0;
    in_out_fds[fds_count - 1] = 1;
    
    // create pipes
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
            setup_redirect_fds(stmt, &in_out_fds[index], &in_out_fds[index+1]);

            dup2(in_out_fds[index], 0);   
            dup2(in_out_fds[index+1], 1);

            close_extra_fds(&in_out_fds[index], &in_out_fds[index + 1]);

            stmt->args->arr[stmt->args->n] = NULL;

            int ret = execvp(stmt->name, stmt->args->arr);
            if (ret == -1)
                err(127, NULL);
        }

        close_extra_fds(&in_out_fds[index], &in_out_fds[index + 1]);
            
        stmt->pid = pid;

        index += 2;
    }

    STAILQ_FOREACH(stmt, list, entries)
    {
        child_pid = stmt->pid;

        int status;
        waitpid(stmt->pid, &status, 0);

        child_pid = 0;
        
        if (WIFEXITED(status))
            return_value = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
        {
            return_value = 128 + WTERMSIG(status);
            fprintf(stderr, "Killed by signal %d.\n", WTERMSIG(status));
        }
        else
            return_value = status;
    }

    free(in_out_fds);
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

    int fd[2];
    int in_out_fd[2] = {0, 1};

    STAILQ_FOREACH(stmt, list, entries)
    {
        if (stmt->internal)
            continue;
        
        --count;

        if (count > 0)
        {
		    int pret = pipe(fd);
            if (pret == -1)
                err(1, "Pipe creation failed");

            in_out_fd[1] = fd[1];
        }

        int pid = fork();
        if (pid == -1)
            err(1, "Fork failed");

        if (pid == 0)
        {
            setup_redirect_fds(stmt, &in_out_fd[0], &in_out_fd[1]);
			
            dup2(in_out_fd[0], 0);
			dup2(in_out_fd[1], 1);

            close_extra_fds(&in_out_fd[0], &in_out_fd[1]);

            stmt->args->arr[stmt->args->n] = NULL;

            int ret = execvp(stmt->name, stmt->args->arr);
            if (ret == -1)
                err(127, NULL);
			exit(1);
		}
		else
        {
            close_extra_fds(&in_out_fd[0], &in_out_fd[1]);
			in_out_fd[0] = fd[0];
            
            child_pid = pid;

            int status;
            waitpid(pid, &status, 0);
            
            child_pid = 0;
            
            if (WIFEXITED(status))
                return_value = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
            {
                return_value = 128 + WTERMSIG(status);
                fprintf(stderr, "Killed by signal %d.\n", WTERMSIG(status));
            }
            else
                return_value = status;
		}
    }
}
