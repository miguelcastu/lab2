//P2-SSOO-22/23

// MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#define MAX_COMMANDS 8


// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];


void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}


/* Timer */
pthread_t timer_thread;
unsigned long  mytime = 0;

void* timer_run ( )
{
	while (1)
	{
		usleep(1000);
		mytime++;
	}
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush (stdin);
			fflush(stdout);
		}
	}

	pthread_create(&timer_thread,NULL,timer_run, NULL);

    /*********************************/

    char ***argvv = NULL;
	int Totalsum=0;

	while (1)
	{
		error: ;
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		// Prompt 
		write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

		// Get command
		//********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
		executed_cmd_lines++;
		if( end != 0 && executed_cmd_lines < end) {
			command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
		}
		else if( end != 0 && executed_cmd_lines == end) {
			return 0;
		}
		else {
			command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
		}
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/

        if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				perror("Error: Maximum number of commands\n");
			}
		}
		for (int i = 0; i < command_counter; i++) {
                  getCompleteCommand(argvv, i);
        }

		// We read the first part of the command in search of "mycalc" or "mytime"
		// MYCALC
		// If "mycalc is found, then..."
        if (strcmp(argv_execvp[0], "mycalc") == 0) {
			// We check for null values on each parameter of the command
            if (argv_execvp[1]==NULL || argv_execvp[2]==NULL || argv_execvp[3]==NULL) {
				// If null is found in the command, we show the error message through
				// The standard output as requested
				if((write(STDOUT_FILENO, "[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n",
                          strlen("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n"))) <0){
					perror ("Error when writing");
					goto error;
				}
                
            } 
			else {
				// If there are no null values...
				// We assign the operands to some int variables
				int op1 = atoi(argv_execvp[1]);
                int op2 = atoi(argv_execvp[3]);
				// Scan the command in position [2] for "add", "mul" or "div"
				// If "add"...
				if (strcmp(argv_execvp[2], "add") == 0) {
					// Compute the sum
					int sum = op1 + op2;
					//Compute the accumulated sum value
                	Totalsum = Totalsum + sum;
					// Store it into the environment "Acc" variable
                	char spc[20];
                	sprintf(spc, "%d", Totalsum);
                	const char *p = spc;
					if (setenv("Acc", p, 1) < 0) {
						perror("Error setting environment variable\n");
						goto error;
					}
					// Char variable to store success message
                    char str[100];
                    snprintf(str, 100, "[OK] %d + %d = %d; Acc %s\n", op1, op2, sum, getenv("Acc"));
					// Display mesage through standard error output as requested
					if((write(STDERR_FILENO, str, strlen(str)))<0){
						perror ("Error when writing");
						goto error;
					}
                }
				// If "mul"...
				else if (strcmp(argv_execvp[2], "mul") == 0) {
					// Compute the product of the multiplication
					int product = op1 * op2;
					// Char variable to store success message
                    char str[100];
                    snprintf(str, 100, "[OK] %d * %d = %d\n", op1, op2, product);
					// Display mesage through standard error output as requested
                    if((write(STDERR_FILENO, str, strlen(str)))<0) {
						perror ("Error when writing");
						goto error;
					}
                }
				// If "div"...
				else if (strcmp(argv_execvp[2], "div") == 0) {
					// Compute the quotient with floor function of division
					int quotient = floor(op1 / op2);
					// Compute remainder
					int remainder = op1 % op2;
					// Char variable to store success message
                    char str[100];
                    snprintf(str, 100, "[OK] %d / %d = %d ; Remainder %d\n", op1, op2, quotient, remainder);
					// Display mesage through standard error output as requested
                    if((write(STDERR_FILENO, str, strlen(str)))<0) {
						perror ("Error when writing");
						goto error;
					}
                }
				else {
					// If mycalc is in the command but there is not "add", "mul" or "div"
					// Error messsage displayed through standard output as requested
                    if((write(STDOUT_FILENO, "[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n",
                          strlen("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n"))) <0) {
					perror ("Error when writing");
					goto error;
					}
                }        
            }
        } 
		// MYTIME
		// If "mytime is found, then..."
		else if (strcmp(argv_execvp[0], "mytime") == 0) {
			// Ckeck that second parameter of the command is NULL
			// So that command is just "mytime \n"
			if (argv_execvp[1] == NULL ) {
				// Use the given timer and divide miliseconds (mytime) by 1000 to get seconds
				unsigned long seconds = mytime / 1000;
				// Minutes are calculated by dividing seconds by 60
				unsigned long minutes = seconds / 60;
				// Hours calculated by dividing minutes by 60
				unsigned long hours = minutes / 60;
				seconds %= 60;
				minutes %= 60;
				// Char variable to store success message
				char str[100];
				// Note that %02lu is used to ensure that the hours, minutes, and seconds are printed as two-digit numbers with leading zeros if necessary
            	snprintf(str, 100, "%02lu:%02lu:%02lu\n", hours, minutes, seconds);
				// Display mesage through standard error output as requested
				if((write(STDERR_FILENO, str, strlen(str)))<0){
					perror ("Error when writing");
					goto error;
				}
			}
			else{
				// Although not requested in statement, an error message is displayed if the command syntax is not "mytime \n"
				if((write(STDOUT_FILENO, "[ERROR] The structure of the command is mytime\n",
                          strlen("[ERROR] The structure of the command is mytime\n"))) <0) {
					perror ("Error when writing");
					goto error;
					}
			}
        } 

		// ****************************************************
		// Simple commands and redirections
		// Redirection of simple commands
		else if (command_counter == 1) {
			//We create a child process
			int pid = fork();

			// Check if the process has been created correctly
			if (pid == -1) {
				perror("Error when creating the child process");
				return (-1);
			}

			int red=0;
			// We do the redirections in the child process
			if (pid == 0) {
				// Redirection of the standard output
				if (strcmp(filev[1], "0") != 0) {
					// Close the standard output
					if((close(1)) <0){
						perror("Error when closing the standard output");
						goto error;
					}
					// Open the file to be redirected on the standard output
					if ((red = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
						perror("Error when opening the file to be redirected\n");
						goto error;
					}
				}
				// Redirection of the standard input
				if (strcmp(filev[0], "0") != 0) {
					//Close the standard input
					if((close(0)) <0){
						perror("Error when closing the standard input");
						goto error;
					}
					// Open the file to be redirected on the standard input
					if ((red = open(filev[0], O_RDWR, 0644)) < 0) {
						perror("Error when opening the file to be redirected\n");
						goto error;
					}
				}
				// Redirection of the standard error
				if (strcmp(filev[2], "0") != 0) {
					if((close(2)) <0){
						perror("Error when closing the standard error");
						goto error;
					}
					// Open the file to be redirected on the standard error
					if ((red = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
						perror("Error when opening the file to be redirected\n");
						goto error;
					}
				}
				// Execution of the child process
				if (execvp(argv_execvp[0], argv_execvp) < 0) {
					perror("Error in the children processes execution\n");
					goto error;
				}
			}
			// Parent process
			else {
				// Close the descriptor redirected
				if(red!=0){
					if((close(red)) <0){
						perror("Error when closing the redirected file");
						goto error;
					}
				}
				// Check if the process is in the background
				if(!in_background) {
					// Wait for the child process to finish
					while (wait(&status) > 0);
					// Check if the child process finished correctly
					if(status < 0) {
						perror("Error in the children processes execution\n");
					}
				}
			}

			}
			// Multiple commands
			else {
				// Create the redirections. Parent and child processes will inherit this redirections.
				int n = command_counter;
				int descriptor[2];
				int pid, status2;
				int red=0;
				int in;
				// Duplicate standard input descriptor
				if ((in = dup(0)) < 0) {
					perror("Error when duplicating descriptor\n");
					goto error;
				}
				for (int i = 0; i < n; i++) {
				// Next pipe is created
				if (i != n - 1) {
					// If it is the last process...
					if (pipe(descriptor) < 0) {
						perror("Pipe error\n");
						exit(0);
					}
				}

				// Next process
				// Fork a child process
				switch (pid = fork()) {
				case -1:
					perror("Error when creating the child process");
					// Close the read end of the pipe
					if((close(descriptor[0])) <0){
						perror("Error when closing the standard input");
						goto error;
					}
					// Close the write end of the pipe
					if((close(descriptor[1])) <0){
						perror("Error when closing the standard output");
						goto error;
					}
					exit(0);
				case 0:
					if (strcmp(filev[2], "0") != 0) {
						// Close the standard error
						if((close(2)) <0){
							perror("Error when closing the standard error");
							goto error;
						}
						// Redirect standard error to a file
						if ((red = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error when opening the file to be redirected\n");
							goto error;
						}
					}

					if (i == 0 && strcmp(filev[0], "0") != 0) {
						// Close the standard input
						if((close(0)) <0){
							perror("Error when closing the standard input");
							goto error;
						}
						// Redirect standard input from a file
						if ((red = open(filev[0], O_RDWR, 0644)) < 0) {
							perror("Error when opening the file to be redirected\n");
							goto error;
						}
					} 
					else {
						// Close the standard output
						if((close(0)) <0){								
							perror("Error when closing the standard output");
							goto error;												
						}
						// Duplicate standard input descriptor
						if (dup(in) < 0) {
							perror("Error when duplicating descriptor\n");
							goto error;
						}
						// Close the duplicated descriptor
						if((close(in)) <0){
							perror("Error when closing descriptor");
							goto error;
						}
					}

					if (i != n - 1) {
						// Close the standard output
						if((close(1)) <0){
									perror("Error when closing the standard output");
									goto error;
							}
						// Duplicate write end of the pipe to standard output
						if (dup(descriptor[1]) < 0) {
							perror("Error when duplicating descriptor\n");
							goto error;
						}
						// Close the read end of the pipe
						if((close(descriptor[0])) <0){
									perror("Error when closing descriptor");
									goto error;
							}
						// Close the write end of the pipe
						if((close(descriptor[1])) <0){
									perror("Error when closing descriptor");
									goto error;
							}
					} else {
						// Check if there is a file redirection for standard output
						if (strcmp(filev[1], "0") != 0) {
							// Close the current standard output (file descriptor 1)
							if((close(1)) <0){
										perror("Error when closing descriptor");
										goto error;
								}
							// Open the file specified in filev[1] for writing
							if ((red = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
								perror("Error when opening the file to be redirected\n");
								goto error;
							}
						}
					}
					
					getCompleteCommand(argvv, i);
						// If the command is intended to run in the background, print the process ID of the current shell process
						if (in_background) {
							printf("[%d]\n", getpid());
						}
						// Execute the command with arguments using execvp()
						if (execvp(argv_execvp[0], argv_execvp) < 0) {
							perror("Execution error\n");
							goto error;
						}
					break;
				default:
					// Close the duplicated descriptor 'in'
					if((close(in)) <0){
								perror("Error when closing descriptor");
								goto error;
						}
						// If not the last command, duplicate the read end of the pipe to 'in' for next command's input                
						if (i != n - 1) {
							// Duplicate the read end of the pipe to 'in'
							if ((in = dup(descriptor[0])) < 0) {
								perror("Error when duplicating descriptor");
								goto error;
							}
							// Duplicate the read end of the pipe again to avoid premature closure
							if (dup(descriptor[0]) < 0) {
								perror("Error when duplicating descriptor");
								goto error;
							}
							// Close the write end of the pipe
							if((close(descriptor[1])) <0){
								perror("Error when closing descriptor");
								goto error;
							}
					}
				}
				}
				if(red!=0){
					// Close the file descriptor used for file redirection
					if((close(red)) <0){
						perror("Error when closing descriptor");
						goto error;
						}
				}
				// If the command is not intended to run in the background, wait for the child process to complete
				if (!in_background) {
					while (wait(&status2) > 0);
					if (stat < 0) {
						perror("Error whith the execution of the child\n");
					}
				}
			}
        }
    return 0;
}