#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

#define MAX_LEN 1024

/*------ Function Prototypes ------*/
char* trim_line(char* line, int length);
char** parse_command(char *line);


int main(int argc, char const *argv[])
{

	size_t buffer_size = MAX_LEN;
	char *line;
	line = (char*) malloc(MAX_LEN*sizeof(char));
	int characters;
	while (1)
	{
		printf("$ ");
		characters = getline(&line, &buffer_size, stdin);
		line = trim_line(line, characters);

		if (strcmp(line, "exit")==0)
			break;

		char** command = parse_command(line);

		if (characters > 1)
		{
			if (strcmp(command[0], "cd") == 0)
			{
				chdir(command[1]);
				continue;
			}

			pid_t pid = fork();
			if (pid > 0)
			{
				//i am the parent
				int status;
				waitpid(pid, &status, 0);
			}
			else if (pid ==0 )
			{
				//i am the child
				if (execvp(command[0], command) == -1)
					execlp("false", "false", NULL);
			}
			else
			{
				perror("Couldn't fork! :-(");
				exit(1);
			}

		}
	}

	return 0;
}

char* trim_line(char* line, int length)
{
	line[length-1]='\0';
	for (int i = 0; i < length; i++)
	{
		if (line[i]=='\t')
			line[i]=' ';
	}
	return line;
}

char** parse_command(char *line)
{

	char** command = malloc(sizeof(char*)*1);
	char* token = strtok(line, " ");
	int count=0;

	while (token)
	{
		count++;
		command=realloc(command, count*sizeof(char*));
		command[count-1]=token;
		token = strtok(NULL, " ");
	}

	command=realloc(command, (count+1)*sizeof(char*));
	command[count]=NULL;


	return command;
}
