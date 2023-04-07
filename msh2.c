//  MSH main file
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
#include <math.h>


#define MAX_COMMANDS 8


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Saliendo del MSH **** \n");
	//signal(SIGINT, siginthandler);
        exit(0);
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

    /*********************************/

    char ***argvv = NULL;
    int num_commands;
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
                }else if( end != 0 && executed_cmd_lines == end)
                    return 0;
                else
                    command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
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

		// We read the first part of the command in search of "mycalc" or "mycp"
		//MYCALC
		//If "mycalc is found, then..."
        if (strcmp(argv_execvp[0], "mycalc") == 0) {
			// We check for null values on each parameter of the command
            if (argv_execvp[1]==NULL || argv_execvp[2]==NULL || argv_execvp[3]==NULL) {
				// If null is found in the command, we show the error message through
				// The standard error output as requested
				if((write(STDERR_FILENO, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n",
                          strlen("[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"))) <0){
					perror ("Error when writing");
					goto error;
				}
                
            } 
			else {
				// If there are no null values...
				// We assign the operands to some int variables
				int op1 = atoi(argv_execvp[1]);
                int op2 = atoi(argv_execvp[3]);
				// Scan the command in position [2] for "add" or "mod"
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
					// Char variable to store succes message
                    char str[100];
                    snprintf(str, 100, "[OK] %d + %d = %d; Acc %s\n", op1, op2, sum, getenv("Acc"));
					// Display mesage through standard error outpt as requested
					if((write(STDERR_FILENO, str, strlen(str)))<0){
						perror ("Error when writing");
						goto error;
					}
                }
				// If "mod"...
				else if (strcmp(argv_execvp[2], "mod") == 0) {
					// Compute the quotient with absolute value of floor function of division
					int quotient = abs(floor(op1 / op2));
					// Compute remainder
					int remainder = op1 % op2;
					// Char variable to store succes message
                    char str[100];
                    snprintf(str, 100, "[OK] %d %% %d = %d ; Quotient %d\n", op1, op2, remainder, quotient);
					// Display mesage through standard error outpt as requested
                    if((write(STDERR_FILENO, str, strlen(str)))<0) {
						perror ("Error when writing");
						goto error;
					}
                } 
				else {
					// If mycalc is in the command but there is neither "add" nor "mod"
					// Error messsage displayed through standard error as requested
                    if((write(STDERR_FILENO, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n",
                          strlen("[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"))) <0) {
					perror ("Error when writing");
					goto error;
					}
                }        
            }
        } 
		//MYCP
		else if (strcmp(argv_execvp[0], "mycp") == 0) {
				//Check for null values or extra parameters in the command	
				if (argv_execvp[1]!=NULL && argv_execvp[2]!=NULL && argv_execvp[3]==NULL) {
					// Char variables for the success message and the contents of the copied file
					char str[100];
					char content[1024];
					// Int variables for the file descriptors
					int orgn_file, copy_file;
					//Open the original file
					if ((orgn_file = open(argv_execvp[1], O_RDONLY, 0644))< 0) {
						// If there is an error opening the original file
						// Error through standard output as requested
						if((write(1, "[ERROR] Error opening original file\n",
                            strlen("[ERROR] Error opening original file\n"))) < 0) {
							perror ("Error when writing");
							goto error;
						}
						goto error;
					}
					if ((copy_file = open(argv_execvp[2], O_TRUNC | O_WRONLY | O_CREAT, 0644))< 0) {
						// If there is an error opening the copied file
						// Error through standard output as requested
						if((write(1, "[ERROR] Error opening original file\n",
                            strlen("[ERROR] Error opening original file\n"))) < 0) {
							perror ("Error when writing");
							goto error;
						}
						goto error;
						// Error closing original file
						if (close(orgn_file)<0){
							perror ("[ERROR] Error closing original file\n");
						}
						goto error;
					}
					int readf;
					//Loop to read the original file
					while ((readf=read(orgn_file,content,sizeof(content)))>0)
					{	//If there is an error writing
						if (write(copy_file,content,readf) <0) {
							// Error closing original file
							if (close(orgn_file)<0){
								perror ("[ERROR] Error closing original file\n");
							}
							// Error closing the copied file
							if (close(copy_file)<0){
								perror ("[ERROR] Error closing the copied file\n");
							}
							perror ("Error when writing");
							goto error;
						}
					}
					// Error closing original file
					if (close(orgn_file)<0){
						perror ("[ERROR] Error closing original file\n");
					}
					// Error closing the copied file
					if (close(copy_file)<0){
						perror ("[ERROR] Error closing the copied file\n");
					}
					// If there is a reading error
					if (readf < 0){
						perror ("Reading error");
						goto error;
					}
					// Displaying success message through standard error output as requested
                    snprintf(str, 100, "[OK] Copy has been successful between %s and %s\n", argv_execvp[1], argv_execvp[2]);
						if (write(STDERR_FILENO, str, strlen(str))<0){
							perror ("Error when writing");
							goto error;
					}
				}
				else {
					//If there are null values the error messsage is displayed
					if((write(1, "[ERROR] The structure of the command is mycp <original_file><copied_file> \n",
                          		strlen("[ERROR] The structure of the command is mycp <original_file><copied_file> \n"))) <0){
						perror("Error when writing");
						goto error;
					}
                }
				// ****************************************************
				// Simple commands and redirections

                } 
				//Redirection of simple commands
				else if (command_counter == 1) {
					//We create a child process
                  	int pid = fork();
                  	if (pid == -1) {
                    	perror("Error when creating the child process");
                    	return (-1);
                  	}

                  	int red=0;
                  	int status;
					//We do the redirections in the child process
                  	if (pid == 0) {
						//Redirection of the standard output
                    	if (strcmp(filev[1], "0") != 0) {
							//Close the standard output
							if((close(1)) <0){
								perror("Error when closing the standard output");
								goto error;
							}
							//Open the file to be redirected on the standard output
							if ((red = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
								perror("Error when opening the file to be redirected\n");
								goto error;
							}
                    	}
						//Redirection of the standard input
                    	if (strcmp(filev[0], "0") != 0) {
							//Close the standard input
							if((close(0)) <0){
								perror("Error when closing the standard input");
								goto error;
							}
							//Open the file to be redirected on the standard input
							if ((red = open(filev[0], O_RDWR, 0644)) < 0) {
								perror("Error when opening the file to be redirected\n");
								goto error;
							}
                    	}
						//Redirection of the standard error
                    	if (strcmp(filev[2], "0") != 0) {
							if((close(2)) <0){
								perror("Error when closing the standard error");
								goto error;
							}
							//Open the file to be redirected on the standard error
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
						//Close the descriptor redirected
						if(red!=0){
							if((close(red)) <0){
								perror("Error when closing the redirected file");
								goto error;
							}
						}
                    	if (!in_background) {
                      		while (wait(&status) > 0);
                      			if (status < 0) {
                        			perror("Error in the children processes execution\n"); // Cambiar todos los errores por perror
                     			}
                    	}
                  	}

                }
				// Multiple commands
				else {
					//Create the redirections. Parent and child processes will inherit this redirections.
                  	int n = command_counter;
                	int descriptor[2];
                  	int pid, status2;
                  	int red=0;
                  	int in;
					if ((in = dup(0)) < 0) {
						perror("Error when duplicating descriptor\n");
						goto error;
					}
                  	for (int i = 0; i < n; i++) {
                    //Next pipe is created
                    if (i != n - 1) {
						// If it is the last process...
                      	if (pipe(descriptor) < 0) {
                        	perror("Pipe error\n");
                        	exit(0);
                      	}
                    }

                    // Next process
                    switch (pid = fork()) {

                    case -1:
                      perror("Error when creating the child project");
											if((close(descriptor[0])) <0){
														perror("Error when closing the standard input");
														goto error;
												}
												if((close(descriptor[1])) <0){
															perror("Error when closing the standard output");
															goto error;
													}
                      exit(0);
                    case 0:

                      if (strcmp(filev[2], "0") != 0) {
												if((close(2)) <0){
															perror("Error when closing the standard error");
															goto error;
													}

												if ((red = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
													perror("Error when opening the file to be redirected\n");
													goto error;
												}
                      }

                      if (i == 0 && strcmp(filev[0], "0") != 0) {
												if((close(0)) <0){
															perror("Error when closing the standard input");
															goto error;
													}
												if ((red = open(filev[0], O_RDWR, 0644)) < 0) {
													perror("Error when opening the file to be redirected\n");
													goto error;
												}
                      } else {
												if((close(0)) <0){
															perror("Error when closing the standard output");
															goto error;
													}
												if (dup(in) < 0) {
													perror("Error when duplicating descriptor\n");
													goto error;
												}
												if((close(in)) <0){
															perror("Error when closing descriptor");
															goto error;
													}
                      }

                      if (i != n - 1) {

												if((close(1)) <0){
															perror("Error when closing the standard output");
															goto error;
													}

												if (dup(descriptor[1]) < 0) {
													perror("Error when duplicating descriptor\n");
													goto error;
												}
												if((close(descriptor[0])) <0){
															perror("Error when closing descriptor");
															goto error;
													}
													if((close(descriptor[1])) <0){
																perror("Error when closing descriptor");
																goto error;
														}
                      } else {
                        if (strcmp(filev[1], "0") != 0) {
													if((close(1)) <0){
																perror("Error when closing descriptor");
																goto error;
														}

													if ((red = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
														perror("Error when opening the file to be redirected\n");
														goto error;
													}
                        }
                      }

                      getCompleteCommand(argvv, i);
											if (in_background) {
	                      printf("[%d]\n", getpid());
	                    }

											if (execvp(argv_execvp[0], argv_execvp) < 0) {
												perror("Execution error\n");
												goto error;
											}
                      break;
                    default:
                

											if((close(in)) <0){
														perror("Error when closing descriptor");
														goto error;
												}                      if (i != n - 1) {
												if ((in = dup(descriptor[0])) < 0) {
													perror("Error when duplicating descriptor");
													goto error;
												}
												if (dup(descriptor[0]) < 0) {
													perror("Error when duplicating descriptor");
													goto error;
												}
												if((close(descriptor[1])) <0){
															perror("Error when closing descriptor");
															goto error;
													}

                      }
                    }
                  }
									if(red!=0){

										if((close(red)) <0){
													perror("Error when closing descriptor");
													goto error;
											}
									}
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