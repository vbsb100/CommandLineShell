/*
Vivek Bharadwaj
CSC345-01
Project 1

*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_LINE 80
#define MAX_HISTORY 100


void forkshell(char *args[], int background){
	int result; //assigned fork value to result	
	if ((result = fork()) == 0){
		//this is the child
		//redirect output to a file
		char* c = args[1];
		int curc = 1;
		int fd;
		int rdyout = 0;
		while(c != NULL) {
			if (strcmp(c, ">") == 0){
				if (args[curc + 1] != NULL){
					fd = open(args[curc + 1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR); //permissions granted for owner
					//standard input to this output file
					dup2(fd, STDOUT_FILENO);
					rdyout = 1;
					args[curc] = NULL;
					c = NULL;
					 	
				}
			}
			else {
				curc++;
				c = args[curc];
			}
		}
		//redirect input from a file
		char* ci = args[1];
		int curci = 1;
		int fdi;
		int rdyin = 0;
		while(ci != NULL) {
			if (strcmp(ci, "<") == 0){
				if (args[curci + 1] != NULL){
					fdi = open(args[curci + 1], O_RDONLY); //permissions granted for owner
					//standard input from a file
					dup2(fdi, STDIN_FILENO);
					rdyin = 1;
					args[curci] = NULL;
					ci = NULL;
					 	
				}
			}
			else {
				curci++;
				ci = args[curci];
			}
		}
		//pipe from output to input for another file
		char* cp = args[1];
		char** cp2 = NULL;
		int curcp = 1;
		int fdp[2]; //declare array for file descriptor for the pipe
		int rdyp = 0;
		while(cp != NULL) {
			if (strcmp(cp, "|") == 0){
				if (args[curcp + 1] != NULL){
					printf("2nd cmd = %s", args[curcp + 1]);
					if(pipe(fdp) < 0){ //if array does not exist, exit
						exit(1);
					}
					//standard piping from output to input
					
					rdyp = 1; 
					args[curcp] = NULL;
					cp = NULL;
					cp2 = &args[curcp + 1];
					 	
				}
			}
			else {
				curcp++;
				cp = args[curcp];
			}
		}
		if (rdyp){
			int pidp;
			if ((pidp = fork()) == 0){
				dup2(fdp[1], STDOUT_FILENO);
				close(fdp[0]); //closes input file
				close(fdp[1]); //closes output file
				execvp(args[0], args);
				exit(0);
			}
			else{
				dup2(fdp[0], STDIN_FILENO);
				close(fdp[0]); //closes input file
				close(fdp[1]); //closes output file
				execvp(cp2[0], cp2); //2nd execution of another command
				wait(&pidp);
				exit(0);
			}

		}
		
		execvp(args[0], args); //executes command
		if (rdyout){
			close(fd); //closes output file
		}
		if (rdyin){
			close(fdi); //closes input file
		}
		
		exit(0); //terminates child process to begin the wait for parent process
	}
	else if (result == -1){
		//this is unsuccessful
		printf("Fork unsuccessful\n");
	}
	else {
		//this is parent
		int status = 0;
		if (background == 0) {
			wait(&status); //wait for child to execute
		}
	}
}





int main(int argc, char *argv[]){

	char *args[MAX_LINE/2 + 1];
	int should_run = 1;
	char buff [MAX_LINE + 1]; //holds whole line argument
	FILE * filePointer;
	int background = 0;
	char **history[MAX_HISTORY];
	int backgroundhistory[MAX_HISTORY];
	int historyidx = -1;
	char cwd [PATH_MAX];
	
	
	if (argc > 1) {
		filePointer = fopen(argv[1], "r"); //if more than 1 argument, read argument	
	}
	else {
		filePointer = stdin; //read from the keyboard arguments
	}
	
	
	while (should_run){
		if (getcwd(cwd, sizeof(cwd)) != NULL){ //gets the current directory
			char *s = &cwd[strlen(cwd) - 1]; //points to last character of the path
			for (int i = strlen(cwd) - 1; i >= 0; i--){
				if (cwd[i] == '/'){ //finds the last / for the working directory
					break;				
				}
				s = &cwd[i]; //s points to the first character of current directory
			} 
			printf("osh:%s>", s);
		}
		else {
			printf("osh>");
		}
		fflush(stdout);
		fgets(buff, MAX_LINE, filePointer); 
		//read a line from the if or else statement involving reading arguments
		//printf("String is the %s\n", buff);
		buff [strlen(buff) - 1] = '\0'; //strips new line character from the end of line
		if (buff[strlen(buff) - 1] == '&'){ //checking for background execution
			background = 1;
			buff [strlen(buff) - 1] = '\0';
		}
		else{
			background = 0;
		}
		char *token = strtok(buff, " "); //command that will be executed
		
		for (int i = 0; i < (MAX_LINE/2 + 1); i++){
			args[i] = NULL; //nullify original arguments
		}
		if (token != NULL && strcmp(token, "!!") != 0 ){ 
		//when !! is not executed, it makes space to save the commands of the arguments
			int i = 0;
			historyidx++;
			history[historyidx] = (char**)malloc((MAX_LINE/2 + 1) *sizeof(char*));
			while (token != NULL && i < (MAX_LINE/2 + 1)) {	
			//looping to get all other parameters that does not exceed the length of 80
				char* mystring = (char*)malloc((strlen(token) + 1)*sizeof(char));
				history[historyidx][i] = mystring;
				history[historyidx][i] = strcpy(history[historyidx][i], token);
				//history array of all command arguments
				//saves every argument passed through the command line
				args[i] = token;
				//printf("args number %d is %s\n", i, args[i]);
				token = strtok(NULL, " "); //
				i++;
			}
			if (strcmp(args[0], "exit") == 0){ 
			//if first argument exit, terminate
				should_run = 0;
			}
			else {
				if (strcmp(args[0], "cd") == 0 && args[1] != NULL){ 
				//if 1st argument cd, 2nd argument is name of directory
					chdir(args[1]);
				}
				
				backgroundhistory[historyidx] = background; 
				//saves the history for the background execution
				forkshell(args, background); //fork
			}
			
		} 
		else {
			if (token != NULL && strcmp(token, "!!") == 0 ){
				forkshell(history[historyidx], backgroundhistory[historyidx]);
			}
		}

		
		
	}
	return 0;
}
