#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

#define MAX_LEN 1024

/*------ Function Prototypes ------*/
char* trim_line(char* line, int length, int* has_pipes);
char** parse_command(char *line);
void cd(char** command);
void create_proccess (char** command);
void create_proccess_with_pipes(char *line);
char** create_array_of_cmds(char *line);

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
		int has_pipes = 0;
		line = trim_line(line, characters, &has_pipes);

		//if user input is exit, then return with success
		if (strcmp(line, "exit")==0)
			break;

		if (!has_pipes) //if there are no pipes in the input act as before
		{
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
		else if (has_pipes > 0)
		{
			create_proccess_with_pipes(line);
		}


	}

	return 0;
}

void create_proccess_with_pipes(char *line)
{
	char** commands = create_array_of_cmds(line);

	// at this point i have an array whom each slot contains a pointer
	// to a pointer of the first part of each command

	// printf("%s\n", commands[0]);
	// printf("%s\n", commands[1]);
	//
	char** cmd1 = parse_command(commands[0]);
	char** cmd2 = parse_command(commands[1]);
	//
	// printf("%s\n", cmd1[0]);
	//
	// printf("%s\n", cmd2[0]);

	pid_t pid = fork();

	if (pid > 0)
	{
		//i am the first parent (the shell)
		waitpid(pid, NULL, 0);
	}
	else if (pid == 0)
	{
		int fd[2];
		pipe(fd);

		pid_t pid2 = fork();

		//command format: left_command | right_command =>
		//command format: cmd1[][] | cmd2[][]

		if (pid2 > 0)
		{
			// I am the second parent, child of the first parent
			// I must wait my child to finish with th left command
			// so I can execute the right one
			dup2(fd[0], 0); //don't read from stdin, replace it with the read end of the pipe
			close(fd[1]);
			close(fd[0]);
			execvp(cmd2[0], cmd2);
			waitpid(pid, NULL, 0);
		}
		else if (pid2 == 0)
		{
			//i am the child of the second parent
			//i must execute the left command. My parent is waiting my output.
			dup2(fd[1], 1); //replace stdout with the write end of the pipe (fd[1])
			close(fd[0]);
			close(fd[1]);
			execvp(cmd1[0], cmd1);
		}
		else
			exit(1);
	}
	else
		exit(1);
}

char** create_array_of_cmds(char *line)
{
	char** commands;

	char* p = line;

	int pipes=0;

	for (int i = 0; i < strlen(line); i++)
	{
		if (line[i]=='|')
		{
			pipes++;
		}
	}

	commands = malloc(sizeof(char*)*(pipes+1));

	char* token = strtok(line, "|");

	int counter=0;
	while ( token != NULL )
	{
		commands[counter]=token;
		counter++;
		token = strtok(NULL, "|");
	}

	return commands;

	/*
	input: "ls -l -a | wc -w"

	*commands[2] =
	--------------------------
	| "ls -l -a " | " wc -w" |
	--------------------------
	*/
}

char* trim_line(char* line, int length, int* has_pipes)
{
	//replace \n with \0
	line[length-1]='\0';

	//replace all tabs with whitespace
	for (int i = 0; i < length; i++)
	{
		if (line[i]=='\t')
			line[i]=' ';
		else if (line[i] == '|')
			*has_pipes = *has_pipes + 1;
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
		if (execvp(command[0], command) == -1)
			exit(1);
	}
	else
	{
		perror("Couldn't fork! :-(");
		exit(1);
	}
}
