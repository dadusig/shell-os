#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

void    loop_pipe(char ***cmd)
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;

  while (*cmd != NULL)
    {
      pipe(p);
      if ((pid = fork()) == -1)
        {
          exit(EXIT_FAILURE);
        }
      else if (pid == 0)
        {
          dup2(fd_in, 0); //change the input according to the old one
          if (*(cmd + 1) != NULL)
            dup2(p[1], 1);
          close(p[0]);
          execvp((*cmd)[0], *cmd);
          exit(EXIT_FAILURE);
        }
      else
        {
          wait(NULL);
          close(p[1]);
          fd_in = p[0]; //save the input for the next command
          cmd++;
        }
    }
}

int main()
{
  char *ls[] = {"ls", NULL};
  char *grep[] = {"sort", "-r", NULL};
  char *wc[] = {"sort", NULL};
  char **cmd[] = {ls, wc, grep, NULL};

  loop_pipe(cmd);
  return (0);
}

// https://stackoverflow.com/questions/8389033/implementation-of-multiple-pipes-in-c
// https://stackoverflow.com/questions/17630247/coding-multiple-pipe-in-c
// https://www.google.gr/search?ei=34z3WbWJOvHb6QTi-LWQAw&q=c+multiple+pipelines+using+one+pipe&oq=c+multiple+pipelines+using+one+pipe&gs_l=psy-ab.3...2467.5307.0.5785.2.2.0.0.0.0.131.255.0j2.2.0....0...1.1.64.psy-ab..0.0.0....0.fMPipPpO0-A
