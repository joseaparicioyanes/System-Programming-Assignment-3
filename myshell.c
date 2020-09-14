/*
************************************************************************************************************************************************
 *Purpose/Description:  	This is a program that uses multi-processes to execute user-entered unix commands. The program reads a command 
			line from the command line interface, entered by the user. Then the program separates them and counts them in blocks 
			divided by pipes (|). Also, the program checks for the following instructions: <, <<,>, >> or &. Once the arguments 
			have been processed, the program calls the execute function to execute the commands. In this function it is checked if 
			the commands are "exit" or "quit", and if so, then the execution is finished. Otherwise, different processes are created 
			using the fork () function. Each one of these processes is then replaced by a new process created with execvp () to execute 
			each one of the commands, and at the end go back to the main function and wait for new instructions.	  

 * Author’s Name: Jose Aparicio
 * Author’s Panther ID: 6186111 
 * Certification: I hereby certify that this work is my own and none of it is the work of any other person.
**************************************************************************************************************************************************
*/
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_ARGS 20											//max number of arguments
#define BUFSIZ2 1024											//size of buffer

int getArgs(char *cmdline, char args[MAX_ARGS][MAX_ARGS], int *numArgs);				//function to separate the arguments
int get_args(char* cmdline, char* args[]) 								//function to verify and count the arguents
{
  	int i = 0;											//int to be use in the for loop

  	
  	if((args[0] = strtok(cmdline, "\n\t ")) == NULL)						//checking for invalid arguments (no args) 
    		return 0; 

  	while((args[++i] = strtok(NULL, "\n\t ")) != NULL) {						//using strtok to look for a substring (the last one is always NULL)
    		if(i >= MAX_ARGS) {
      			printf("Too many arguments!\n");
      			exit(1);
    		}
  	}
  	return i;											//returning i (num of args)
}


int execute(char *cmdline){										//function to execute the commands
	int i, pid,pid2,pidStatus,fd[2];								//int variable for for loop, pids, etc
	char* cmd[MAX_ARGS];                           							//first command
	char *cmd1[MAX_ARGS];										//second command
	char args[MAX_ARGS][MAX_ARGS];									//all comands
	int numArgs;											//number of commands
	pipe(fd);											//piping
	int flag = getArgs(cmdline,args, &numArgs);							//setting the flag, args, and numArgs
	
	if(strcmp(args[0], "quit ")== 0 || strcmp(args[0], "exit ") == 0) {				//checking for exit or quit
    		exit(0);
  	}
	if(flag==0){											//if flag =0 there are at least ine command
		get_args(args[0], cmd);
		if(numArgs>0)
			get_args(args[1], cmd1);							//getting the argument
		switch (pid = fork()){									//switch statement for the first child

			case 0: 									//case 0 (child 1) executes cmd
				if(numArgs>0){
					dup2(fd[1], 1);	                                		//making this end of the pipe the standard output
					close(fd[0]); 		                        		//closing the other end
				}
				execvp(cmd[0], cmd);	                        			// running the command
				printf("ERROR. Invalid command or system error! Please try again.\n");	//error message

			case -1:                                                			//if pid is less than 0, there is an error
				fprintf(stderr, "Not able to complete the task! Please try again\n");	//printing an error message for the user
				exit(1);	
		}

		switch (pid2 = fork()){									//switch statement for the second child
			case 0:										//case 0 (child 1) executes cmd
				dup2(fd[0], 0);								//making this end of the pipe the standard input
				close(fd[1]);								//closing the other end
				execvp(cmd1[0], cmd1);							// running the command
				printf("ERROR. Invalid command or system error! Please try again.\n");	//error message

			case -1:									//if pid is less than 0, there is an error
				fprintf(stderr,"Not able to complete the task! Please try again.\n");	//printing an error message for the user
				exit(1);
		}
	close(fd[0]); 											//closing
	close(fd[1]);											//closing
	
	while ((pid2 = wait(&pidStatus)) != -1);							//waiting for all processes to finish
	}else{
		pid=fork();										//creting a new process
		if(pid==0){
			execlp( "/bin/sh", "/bin/sh", "-c", args[0], (char *)NULL );			//executing the command
		}else{
			wait(NULL);									//waiting for the process to finish
		}
	}
				
	
	return 0;											//retuning 0 if success
}

int main (int argc, char* argv [])									//main function
{
  	char cmdline[BUFSIZ2];										//char array to store the commands

  	for(;;) {											//infinite for loop
    		printf("COP4338$: ");
    		if(fgets(cmdline, BUFSIZ, stdin) == NULL) {						//getting the cmd line 
      			perror("fgets failed");								//erros message
      			exit(1);									//killing the program
    		}
    		execute(cmdline);									//calling the function to execute the comand
  	}
  	return 0;
}

int  getArgs(char *cmdline, char args[MAX_ARGS][MAX_ARGS], int *numArgs){				//function to separate the arguments
	char *arguments[MAX_ARGS];									//char array to store the arguments
	int i,async, nargs = get_args(cmdline, arguments);						//int variables for the loop, number of args, etc
        if(nargs <= 0) return;										// if no args return 0
	int counter=0;											//variable to count the number of blocks of arguments (separated by |)
	int flag=0;											//flag to know if there is at least one argument
	memset(args,0,(MAX_ARGS*sizeof(args[0])));							//cleanning the array
	for(i=0;i<nargs;i++){										//for loop to iterate through the array of arguments
		if(strcmp(arguments[i],"|")==0){							//increasing the counter if | is found
			counter++;									//increasing the counter
		}
		
		if(strcmp(arguments[i], "<")==0 || strcmp(arguments[i], "<<")==0  || strcmp(arguments[i], ">")==0 || strcmp(arguments[i], ">>")==0 || counter > 1 || strcmp(arguments[i], "&")==0){ /*searching for < << > >> or & */
			flag=1;										//setting flag to true
			for(i=0;i<nargs;i++){								//for loop to iterate through the characters of the argument
                		strcat(args[0],arguments[i]);						//concatenating the arguments
				strcat(args[0]," ");							//adding a space
			}
			i=nargs;									//setting i to nargs to terminate the loop
			counter=0;									//setting counter to 0
		}
	}
	
	if(flag==0){											//checking the flag
		int count=0;										//variable to set the index of the array
		for(i=0;i<nargs;i++){									//for loop to iterate through the arguments array
			if(strcmp(arguments[i],"|")==0){						//checking for |
				strcat(args[count],"\0\t\n");						//adding null terminator + spaces + new line 
				count++;								//incrementing count
				i++;									//incrementing i
			}
	                strcat(args[count],arguments[i]);						//concatenating the strings
                        strcat(args[count]," ");							//adding a space
                }
	}
	*numArgs=counter;										//setting numArgs equal to counter
	return flag;											//returning flag
}
