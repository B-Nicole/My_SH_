
/**
	Brittany Hughes
	Created: December 5, 2022

	This code we will create a working shell
	in order to recognize certain patterns to set flags and create output
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //return true false
#include <signal.h>	 // sigaction()
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include <unistd.h>
#include <fcntl.h> // to open and alter files with permissions
#include <ctype.h>
#include <errno.h>

#include "myShell.h"
#include "getword.h"

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c
#define dbg(fmt, ...)                                 \
	do                                                \
	{                                                 \
		fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
				__LINE__, __func__, __VA_ARGS__);     \
	} while (0)

//************************************************************
// 					GLOBAL VARIABLES
//************************************************************
int getValue = 0; // gets the value returned by getWord()

char s[STORAGE * MAXITEM]; // want a char array to store 100 words that may be 255 ch long
char *cmd[MAXITEM];		   // pointer array

char lastWord[STORAGE * MAXITEM]; // stores the last word from the last command

// for getting the location of files for output and input redirection
char *outptr;
char *inptr;

int numOfElements = 0;
int historyIndex = 0;
int digit = 0; // for the history command

// FGETS LEAVES -1
//  checking for certain conditions such as new line, end of file and empty
int EMPTY = 0;
int NEW_LINE = 0;
int END = 0;
int EndOFF = 0;

// checking for certain commands
int CD_FLAG = 0;

//****************
//   META FLAGS
//****************
int LEFT_ARROW = 0;	  // <
int RIGHT_ARROW = 0;  // >
int AMPERSAND = 0;	  // &
int PIPE = 0;		  // |
int EXCLAMATION = 0;  // !
int RR_ARROW_AMP = 0; // >>&
int R_ARROW_AMP = 0;  // >&
int RR_ARROW = 0;	  // >>
int BACK_PIPE = 0;	  // \|
int pipeLocation = 0; // location of the pipe

//****************
// PROCESSES, SIGNALS and PIPES
//****************
pid_t pid, Child_Grandchild_PID;
int sig = 0;

//********************************************************************************/
//						SIGNAL HANDLER FUNCTON myhandler()
//                Catches the signal so we wont terminate early
//********************************************************************************/
void myhandler(int signum)
{

	signal(SIGINT, SIG_DFL);
	sig = 1;
}
//********************************************************************************/
//								RESET() FUNCTION
//********************************************************************************/

void reset()
{

	getValue = 0;
	numOfElements = 0;

	// checking for certain conditions such as new line, end of file and empty
	EMPTY = 0;
	NEW_LINE = 0;
	END = 0;
	EndOFF = 0;

	// checking for certain commands
	CD_FLAG = 0;

	//****************
	//   META FLAGS
	//****************
	LEFT_ARROW = 0;	  // <
	RIGHT_ARROW = 0;  // >
	AMPERSAND = 0;	  // &
	PIPE = 0;		  // |
	EXCLAMATION = 0;  // !
	RR_ARROW_AMP = 0; // >>&
	R_ARROW_AMP = 0;  // >&
	RR_ARROW = 0;	  // >>
	pipeLocation = 0; // location of the pipe

	BACK_PIPE = 0;

	return;
}

//********************************************************************************
//								PARSE() FUNCTION
//********************************************************************************
/*	Parse()
	ParamsL Takes in a pointer *w that is fed into getword() from getword.c
	getword() returns a number which contains the size of the string

	We continue to go through getword() in a loop until getValue which contains the value recieved
	from getword() is -1 which indicates EOF

	In the loop we set various flags using the global variables.
		If we encounter cd that indicates change directoru
		If we encounter end. and numOfElements (That counts the number of elements we have encountered so far) is 0 meaning
			that it is the first occurance
		1 means yes
		0 means no

	We also check for metacharacters such as <, >, &, >>, >&, >>&, !!, !, # and |.
		We set the flags to 1 if we encounter them
		We set the flags to 0 if we dont encounter them

***************************************************************************************/

void parse(char *w) // takes in the address
{

	size_t Address = 0;
	while (getValue != -1) // we wll send w to get word. When we pass in to get word, we are passing in the address location.
	{

		getValue = getword(w); // get value will retain a number either -1 for EOF or end. or the size of the string
		if (getValue == -1)
		{

			EndOFF = 1;
			return;
		}

		if (getValue == -5)
		{ // we encountered a \| so we will set the flag to 1 and grab the next word
			BACK_PIPE = 1;
			getValue = getword(w);
		}

		if (getValue == 0)
		{

			if (getValue == 0 && numOfElements == 0)
			{
				EMPTY = 1;
				return;
			}
			return;
		}

		if (strchr(w, '#') != NULL)
		{ // signals to ignore the rest of the line if we encounter a #
			while (getValue != 0)
			{
				getValue = getword(w);
			}
			cmd[Address] = NULL;
			return;
		}

		if ((strcmp(w, "end.") == 0 && Address == 0))
		{ // signals to cut the program
			END = 1;
			return;
		}

		if (strcmp(w, "cd") == 0)
		{ // signals to change directory

			CD_FLAG = 1;
			cmd[Address] = w;
			Address++;
			AMPERSAND = 0;
		}
		else if (strcmp(w, ">") == 0)
		{ // signals to redirect output
			RIGHT_ARROW++;
			getValue = getword(w);

			outptr = w;
			AMPERSAND = 0;
		}
		else if (strcmp(w, ">&") == 0)
		{ // redirect standard output and standard error
			getValue = getword(w);
			outptr = w;
			R_ARROW_AMP = 1;
			AMPERSAND = 0;
		}
		else if (strcmp(w, ">>&") == 0)
		{ // append standard error and output
			getValue = getword(w);
			outptr = w;
			RR_ARROW_AMP = 1;
			AMPERSAND = 0;
		}
		else if (strcmp(w, ">>") == 0)
		{ // signal to append at the end of the file
			getValue = getword(w);
			outptr = w;
			RR_ARROW = 1;
			AMPERSAND = 0;
		}
		else if (strcmp(w, "<") == 0)
		{ // signal to redirect standard
			getValue = getword(w);
			inptr = w;

			LEFT_ARROW = 1;
			AMPERSAND = 0;
		}
		else if (strcmp(w, "!$") == 0)
		{ // signal to run the last word of the last command
			cmd[Address] = lastWord;
			getValue = strlen(lastWord);
			Address++;
		}

		else if (strcmp(w, "|") == 0)
		{						 // if pipe we will prepare cmd for execution
			cmd[Address] = NULL; // we first add a null to the end of the array so that execvp knows where to stop for the first execution
			Address++;			 // We increment to the next slot of the array

			getValue = getword(w);
			cmd[Address] = w;

			pipeLocation = Address; // we store the location of the pipe
			Address++;

			PIPE++; // we increment the pipe flag and if its greater than 1, it will trigger an error
			AMPERSAND = 0;
		}
		else if (strcmp(w, "&") == 0)
		{
			AMPERSAND = 1; // signals to run in the background
		}
		else
		{
			cmd[Address] = w;	 // add the w from getWord() to the cmd
			strcpy(lastWord, w); // we store the last word of the last command
			Address++;			 // move to the next element in the array
			AMPERSAND = 0;		 // no ampersand was found
		}
		// printf("BOTTOM OF PARSE: cmd[%d] = %s\n", Address, cmd[Address]);
		getValue++;		 // we increment get value to account for the null terminator
		w += getValue;	 // we move the pointer to the next word
		numOfElements++; // we increment the number of elements in the array
	}
	cmd[Address] = NULL; // we add a null terminator to the end of the array for execvp to process

	return;
}

//********************************************************************************
//								CHANGE_DIRECTORY() FUNCTION
//********************************************************************************
/*   
***************************************************************************************/
void CHANGE_DIRECTORY()
{

	if (numOfElements == 1) // If its just cd and nothing else
	{

		if (chdir(getenv("HOME")) != 0)
		{
			fprintf(stderr, "chdir() to $HOME failed\n");
			continue;
		}
		else
		{
			printf("%s\n", getcwd(s, 100));
			continue;
		}
	}
	else if (numOfElements > 2) // if there is more than two elements EX: cd directory1 directory2
	{
		fprintf(stderr, "Too many elements\n");
		continue;
	}
	else
	{
		if (chdir(cmd[1]) != 0)
		{
			fprintf(stderr, "No such file or directory in CHDIR.\n");
			continue;
		}
		else
		{
			printf("%s\n", getcwd(s, 100));
			continue;
		}
	}
}
//********************************************************************************
//								REDIRECT_STDOUT() FUNCTION
//********************************************************************************
/*     Param: int fd
***************************************************************************************/
void REDIRECT_STDOUT(int fd)
{
	if (dup2(fd, STDOUT_FILENO) < 0)
	{
		close(fd);
		fprintf(stderr, "Error : Can not execute dup().\n");
		exit(EXIT_FAILURE);
	}
}
//********************************************************************************
//								REDIRECT_STDERR() FUNCTION
//********************************************************************************
/*		Param: int fd
***************************************************************************************/
void REDIRECT_STDERR(int fd)
{
	if (dup2(fd, STDERR_FILENO) < 0)
	{
		close(fd);
		fprintf(stderr, "Error : Can not execute dup2().\n");

		exit(EXIT_FAILURE);
	}
}
//********************************************************************************
//								READ_TO_INPUT() FUNCTION
//********************************************************************************
/*		Param: None
***************************************************************************************/
void READ_TO_INPUT()
{
	if ((fd = open(inptr, O_RDONLY)) < 0) // O_RDONLY: Read only
	{
		fprintf(stderr, "Error: File does not exist\n");
		exit(EXIT_FAILURE);
	}
	dup2(fd, STDIN_FILENO);

	if (close(fd) < 0)
	{
		fprintf(stderr, "Error: Cannot close file\n");
		exit(EXIT_FAILURE);
	}
	execvp(cmd[0], cmd);
	fprintf(stderr, "Error: Command not found\n");
	exit(EXIT_FAILURE);
}
//********************************************************************************
//								REDIRECT_OUTPUT_TO_FILE()
//********************************************************************************
/*
***************************************************************************************/
void REDIRECT_OUTPUT_TO_FILE()
{

	// The argument flags must include one of the following access modes: O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the
	// file read-only, write-only, or read/write, respectively.

	// O_APPEND - Append to the end of the file
	// O_CREAT - Create the file if it doesn't exist
	// O_EXCL - Only create the file if it doesn't exist

	int fd = open(outptr, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR); // write only
	if (fd < 0)
	{
		fprintf(stderr, "Error : Cannot overwrite existing file.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		// print STDOUT to screen
		// printf("WHAT SHOULD BE IN FILE: %s\n", STDERR_FILENO);
		int status = dup2(fd, STDOUT_FILENO);
		if (status < 0)
		{
			fprintf(stderr, "Error : Can not execute dup().\n");
			exit(EXIT_FAILURE);
		}

		execvp(cmd[0], cmd);
	}
	if (close(fd) < 0)
	{
		fprintf(stderr, "Error: Cannot close file\n");
		exit(EXIT_FAILURE);
	}
	// exit(1);
	continue;
}
//********************************************************************************
//								LEFTARROW()
//********************************************************************************
/*
***************************************************************************************/
void LEFTARROW()
{
	int fd;
	if ((fd = open(inptr, O_RDONLY)) < 0)
	{
		fprintf(stderr, "Error: File does not exist\n");
		exit(EXIT_FAILURE);
	}
	dup2(fd, STDIN_FILENO);

	if (close(fd) < 0)
	{
		fprintf(stderr, "Error: Cannot close file\n");
		exit(EXIT_FAILURE);
	}
	execvp(cmd[0], cmd);
	fprintf(stderr, "Error: Command not found\n");
	exit(EXIT_FAILURE);
}
//********************************************************************************
//								RIGHTARROW()
//********************************************************************************
/*
***************************************************************************************/
void RIGHTARROW()
{
	// Take the output of a command and redirect it into a file (will overwrite the whole file)

	// The argument flags must include one of the following access modes: O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the
	// file read-only, write-only, or read/write, respectively.

	// O_APPEND - Append to the end of the file
	// O_CREAT - Create the file if it doesn't exist
	// O_EXCL - Only create the file if it doesn't exist

	int fd = open(outptr, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR); // write only
	if (fd < 0)
	{
		fprintf(stderr, "Error : Cannot overwrite existing file.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		int status = dup2(fd, STDOUT_FILENO);
		if (status < 0)
		{
			fprintf(stderr, "Error : Can not execute dup().\n");
			exit(EXIT_FAILURE);
		}

		execvp(cmd[pipeLocation], (cmd + pipeLocation));
	}
	if (close(fd) < 0)
	{
		fprintf(stderr, "Error: Cannot close file\n");
		exit(EXIT_FAILURE);
	}
}
//********************************************************************************
//								RIGHTARROW_RIGHTARROW()
//********************************************************************************
/*
***************************************************************************************/
void RIGHTARROW_RIGHTARROW()
{

	fd = open(outptr, O_WRONLY | O_APPEND); // WRITING ONLY IF IT EXISTS

	if (fd < 0)
	{
		close(fd);
		fprintf(stderr, "Error : Cannot overwrite existing file.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		REDIRECT_STDOUT();
		execvp(cmd[pipeLocation], (cmd + pipeLocation));

		if (close(fd) < 0)
		{
			fprintf(stderr, "Error: Cannot close file\n");
			exit(EXIT_FAILURE);
		}

		continue;
	}
}
//********************************************************************************
//								RIGHTARROW_AMPERSAND()
//********************************************************************************
/*
***************************************************************************************/
void RIGHTARROW_AMPERSAND()
{
	int fd = open(outptr, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd < 0)
	{
		fprintf(stderr, "Error : File already exist.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		REDIRECT_STDOUT();
		REDIRECT_STDERR();

		execvp(cmd[pipeLocation], (cmd + pipeLocation));

		if (close(fd) < 0)
		{
			fprintf(stderr, "Error: Cannot close file\n");
			exit(EXIT_FAILURE);
		}
	}
}
//********************************************************************************
//								RIGHTARROW_RIGHTARROW_AMPERSAND()
//********************************************************************************
/*
***************************************************************************************/
void RIGHTARROW_RIGHTARROW_AMPERSAND()
{

	fd = open(outptr, O_WRONLY | O_APPEND);
	if (fd < 0)
	{
		fprintf(stderr, "Error : File does not exist.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		REDIRECT_STDOUT();
		REDIRECT_STDERR();
		execvp(cmd[pipeLocation], (cmd + pipeLocation));

		if (close(fd) < 0)
		{
			fprintf(stderr, "Error: Cannot close file\n");
			exit(EXIT_FAILURE);
		}
		continue;
	}
}

//********************************************************************************
//								MAIN FUNCTION
//********************************************************************************
/*
	gcc -g -Wall -c -o p2.o p2.c && gcc p2.o getword.o -o p2

	This runs in an endless for loop processing the flags declared globally and
	calls the parse functon above inorder to set those flags.

	We call the prompt for the user %1% and for each non empty line we increment the integer by one
		print("%%{promptValue}%% ", promptValue);

	At the beginning it processes change directory and the flags, END, EMPTY, NEWLINE...
	If those flags are not set it continues and forks() a process.

	When we get the signal to terminate it should print out ("p2 terminated");

	LEFT_ARROW (<)
	RIGHT_ARROW (>)
	RR_ARROW (>>)
	RR_ARROW_AMP (>>&)
	R_ARROW_AMP (>&)
	AMPERSAND (&)
	PIPE (|)
	EXCLAMATION (!)

//********************************************************************************/
int main(int argc, char **argv)
{

	setpgid(0, 0);
	signal(SIGTERM, myhandler); //catches the signal so we can ignore it 

	int pipefd[2];
	char *result;
	int fd, i;

	int isArgC = 0;		 // flag where if argc is greater than 1. We will read from the file and NOT print the prompt or "p2 terminated"
	int promptValue = 1; // will be used to increment the shells prompt

	for (;;)
	{

		// we reset all flags and "clear" the arrays
		reset();
		memset(s, 0, sizeof(s));
		memset(cmd, 0, sizeof(cmd));

		if (argc > 1) // we will read from the file
		{

			if ((fd = open(argv[1], O_RDONLY)) < 0) // opening the file for read only error checking
			{
				fprintf(stderr, "ERROR: Cannot open file.\n");
				exit(EXIT_FAILURE);
			}
			else
			{

				while (read(fd, s, sizeof(s)) > 0)
				{ // we will empty and reset the global variables and arrays each iteration and store file contents in history before parsing
					reset();
					memset(s, 0, sizeof(s));
					memset(cmd, 0, sizeof(cmd));

					parse(s);
				}
				if (close(fd) < 0)
				{ // file closing error checking
					fprintf(stderr, "Error: Cannot close file (RIGHT ARROW)\n");
					exit(EXIT_FAILURE);
				}
			}
			isArgC = 1;
		}

		else
		{

			printf("%%%i%% ", promptValue); //prompt

			if ((result = fgets(s, (MAXITEM * STORAGE), stdin)) == NULL)
			{
				EndOFF = 1;
			}
			else
			{

				ungetc('\n', stdin);
				for (i = strlen(s) - 2; i >= 0; i--)
				{
					ungetc(s[i], stdin);
				}

				parse(s); // sending the user input to the above function
			}
		}

		if (END || EndOFF)
			break;

		/**
		 *	If empty we continue the loop without
		 *	incrementing the prompt. If not empty we increment the prompt
		 */
		if (EMPTY == 1)
			continue;
		else
			promptValue++;

		/**
		 * We first check if we have the change directory flag set
		 * If the cd flag is set and the number of elements is equal to 1, there is no other arguments and we
		 * change to the $HOME directory else we change to the directory specified by the user
		 *
		 * If the cd flag is set and the number of elements is greater than 1, we print out an error message
		 *
		 */
		if (CD_FLAG)
			CHANGE_DIRECTORY();

		if (PIPE > 1)
		{ // error checking multiple pipe input
			fprintf(stderr, "ERROR: Too many pipes.\n");
			continue;
		}

		/**
		 * we fflush the stdout and stderr before we fork a child process
		 */
		fflush(stdout);
		fflush(stderr);
		pid = fork();

		/**
		 * If the pid is less than 0, we print out an error message
		 * If the pid is equal to 0, we execute the command as the child process
		 * If the pid is greater than 0, we are the parent
		 */
		if (pid < 0)
		{
			fprintf(stderr, "Error : Program did not fork CHILD correctly.\n");
			exit(EXIT_FAILURE);
		}

		else if (pid == 0)
		{

			/**
			 * If there is a pipe we need to create a pipe and then fork again to create a grandchild process
			 * created by the child process
			 *
			 * The child process will be reading from the right hand side of the pipe and the grandchild will be
			 * writing to the left hand side of the pipe
			 * To determine the right and left we use the location of | in the cmd[] to split into two commands
			 */
			if (PIPE && (BACK_PIPE != -5))
			{

				/**
				 * Before we fork we must fflush stdout and stderr
				 * We can now pipe and fork the grandchild
				 */
				pipe(pipefd);
				fflush(stdout);
				fflush(stderr);
				Child_Grandchild_PID = fork(); // will give us the id of the child or grandchild

				if (Child_Grandchild_PID < 0) // error handling to see if we forked our grandchild
				{
					fprintf(stderr, "Error : Program did not fork GRANDCHILD correctly.\n");
					exit(EXIT_FAILURE);
				}
				else if (Child_Grandchild_PID == 0)
				{

					// INSIDE GRANDCHILD BRANCH. handle < redirection
					close(pipefd[0]);
					dup2(pipefd[1], STDOUT_FILENO);
					close(pipefd[1]);

					if (LEFT_ARROW) // CHECK: WE WANT TO READ THE CONTENTS OF A FILE INTO THE INPUT OF THE COMMAND
						LEFT_ARROW();

					execvp(cmd[0], cmd);
					fprintf(stderr, "Failed to execute (GRANDCHILD)\n");
					exit(EXIT_FAILURE);
				}
				else
				{

					close(pipefd[1]);
					dup2(pipefd[0], STDIN_FILENO);
					close(pipefd[0]);

					if (RIGHT_ARROW)
						RIGHT_ARROW();

					if (RR_ARROW) // open file for appending and write output to it without overwriting
						RIGHTARROW_RIGHTARROW();

					if (RR_ARROW_AMP) // append STDOUT and STDERR to the end of the file // if not exist fail >>&
						RIGHTARROW_RIGHTARROW_AMPERSAND();

					if (R_ARROW_AMP) // >&
						RIGHTARROW_AMPERSAND();

					execvp(cmd[pipeLocation], (cmd + pipeLocation));
					fprintf(stderr, "Failed to execute (CHILD)\n");
					exit(EXIT_FAILURE);
				}
			}

			if (LEFT_ARROW) // WE WANT TO READ THE CONTENTS OF A FILE INTO THE INPUT OF THE COMMAND
				READ_TO_INPUT();

			if (RIGHT_ARROW) // Take the output of a command and redirect it into a file (will overwrite the whole file)
				REDIRECT_OUTPUT_TO_FILE();

			if (RR_ARROW) // open file for appending and write output to it without overwriting
				RIGHTARROW_RIGHTARROW();

			if (RR_ARROW_AMP) // append STDOUT and STDERR to the end of the file // if not exist fail >>&
				RIGHTARROW_AMPERSAND();

			if (R_ARROW_AMP) // >&
				RIGHTARROW_AMPERSAND();

			if (execvp(cmd[0], cmd) == -1)
			{
				fprintf(stderr, "Error Down here: Can not execute command.\n");
				exit(EXIT_FAILURE);
			}
		}
		/**
		 * The parent wil print the argv[0] of the child followed by
		 * the PID of the child in brackets followed by the normal prompt to process the next command
		 *
		 * Else if not an & we shall wait for all of the children to terminate to prevent
		 * zombies
		 *
		 */
		else if (pid > 0)
		{

			if (AMPERSAND)
			{
				printf("%s[%ld]\n", cmd[0], (long)getpid());
				continue;
			}
			else
			{
				while ((pid = wait(NULL)) > 0)
					;
			}
		}
	}

	killpg(getpgrp(), SIGTERM);
	if (isArgC == 0)
	{ // from the above flag. If we are not reading from a file this will execute
		printf("shell terminated.\n");
	}
	exit(0);
}
