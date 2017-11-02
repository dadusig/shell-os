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
void cd(char** command);
void create_proccess (char** command);

int main(int argc, char const *argv[])
{
	//getline parameters
	size_t buffer_size = MAX_LEN;
	char *line;
	line = (char*) malloc(MAX_LEN*sizeof(char));
	int characters;

	while (1)
	{
		printf("$ ");

		//read user input
		//getline is much more sophisticated than fgets
		characters = getline(&line, &buffer_size, stdin);

		//convert user input to the appropiate form
		line = trim_line(line, characters);

		//if user input is exit, then return with success
		if (strcmp(line, "exit")==0)
			break;

		//split user input into tokens and convert it into an array of strings
		char** command = parse_command(line);

		//if user gave more than one character (other than \n)
		if (characters > 1)
		{
			//if user input is cd
			if (strcmp(command[0], "cd") == 0)
			{
				cd(command);
				continue;
			}
			else //if user input is not a built-in
			{
				//let the fork begin
				create_proccess(command);
			}

		}
	}

	return 0;
}

char* trim_line(char* line, int length)
{
	//replace \n with \0
	line[length-1]='\0';

	//replace all tabs with whitespace
	for (int i = 0; i < length; i++)
	{
		if (line[i]=='\t')
			line[i]=' ';
	}
	return line;
}

char** parse_command(char *line)
{
	//allocate some memory for the command
	char** command = malloc(sizeof(char*)*1);

	//split line into tokens, using space as the delimiter character
	char* token = strtok(line, " ");
	int count=0;

	while (token)
	{
		count++;
		//allocate memory for the next value
		command=realloc(command, count*sizeof(char*));

		//put the pointer to the token, to the just allocated space
		command[count-1]=token;

		//continue tokenize
		token = strtok(NULL, " ");
	}

	//allocate one extra value for the null character
	command=realloc(command, (count+1)*sizeof(char*));
	command[count]=NULL;

	return command;
}

void cd(char** command)
{
	//cd without any parameters => go to home
	if (command[1] == NULL)
	{
		chdir(getenv("HOME"));
	}
	else
	{
		//go to the desired directory
		if ( chdir(command[1]) == -1 )
			perror("cd failed");
	}
}

void create_proccess (char** command)
{
	pid_t pid = fork();
	if (pid > 0)
	{
		//i am the parent
		int status;
		waitpid(pid, &status, 0); //wait for the child
	}
	else if (pid ==0 )
	{
		//i am the child
		//exec the desired, command with its parameters
		//if the command failed, do nothing
		if (execvp(command[0], command) < 0)
			exit(1);
	}
	else
	{
		perror("Couldn't fork! :-(");
		exit(1);
	}
}
