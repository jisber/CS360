//Jacob Isber lab7. In this lab we created a shell script to handel certain functions. Philip helped me with the jsh1 and some of jsh2.
//Gradescript 18 is the one that goes wrong alot bit it is correct. 
//Beta 1.4 (Fixed Memory Leaks)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "fields.h"
#include "jrb.h"

//Start of main
int main(int argc, char ** argv)
{

	//Declaired variables. 
	int i, status,j, count = 0, id, waitVal, ampTrigger = 0, check = 0;
	IS is = new_inputstruct(NULL);
	char *path;
	char **argvpath;
	char *argv1 = argv[1];
	int read, write;

	//If argc > 2 error check
	if(argc > 2) 
	{ 
		fprintf(stderr, "usage: jsh1 [prompt]\n");
		exit(1);
	}

	//if argv[1] is null
	if(argv[1]==NULL)  
		printf("jsh: "); 

	else if(strcmp(argv1, "-")!=0) 
		printf("%s: ", argv1); 

	//Start of while loop
	while(get_line(is) >= 0)
	{
		
		if(argv[1]==NULL)  
			printf("jsh: ");  

		else if(strcmp(argv1, "-")!=0) 
			printf("%s: ", argv1);

		//Resets all counters and triggers
		count = 0;
		ampTrigger = 0;
		read = -1; 
		write = -1;
		j = 0;  
		check = 0; 

		if(is->NF > 0)
		{
			//Grabs path from the fields
			path = is->fields[0];
			//Mallocs the necessary about of size. 
			argvpath = (char**) malloc(sizeof(char*)*is->NF + 1);
			for(i = 0; i < is->NF; i++) 
			{	
				//If it is an & set the trigger else load the path into argvpath
				if(strcmp(is->fields[i], "&")==0)
				{
					ampTrigger = 1;
				}
				else
				{ 
					argvpath[count] = is->fields[count];
					count++;
				}
			}

			//This for loop then goes through the fields again and checks for "<", ">", ">>"
			for(i=0; i<is->NF; i++){
				if(argvpath[i] == NULL) break;
				
				if(strcmp(argvpath[i], "<")==0)
				{	
					//Reads in the path
					read = open(argvpath[i+1], O_RDONLY);
					argvpath[i] = NULL; 
					i++;
					argvpath[i] = NULL;
					check = 1;
				}			
				else if(strcmp(argvpath[i], ">")==0)
				{	
					//Writes to the path
					write = open(argvpath[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
					argvpath[i] = NULL;
					i++;
					argvpath[i]= NULL; 
					check = 1;
					
				}
				
				else if(strcmp(argvpath[i], ">>")==0)
				{	
					//Also will append and write to the path
					write = open(argvpath[i+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
					argvpath[i] = NULL; 
					i++;
					argvpath[i] = NULL;
					check = 1;
				}
				else
				{	
					//Else just load the fields[i] into argvpath
					argvpath[j] = is->fields[i]; 
					j++;  
				}
			}

			//sets the parent id and forks. 
			id = fork();
			//If the id is on the child process
			if(id==0)
			{
				if(check == 1)//If the check is triggered
				{	
					//If the read is valid proceed to dup2
					if(read != -1) dup2(read, 0); 
					//Close read
					close(read);

					//If the write is valid proceed to dup2
					if(write != -1) dup2(write,1);
					//Close write
					close(write);
				}
				//Exectute execvp on argvpath[0]
				execvp(argvpath[0], argvpath);
				perror(argvpath[0]);
				exit(1);
			}
			//Else if we are no longer in the child process and the & trigger has not checked 
			else if(ampTrigger == 0)
			{	
				//We wait and kill all zombie processes. 
				waitVal = wait(&status);
				while(waitVal != id)
				{
					waitVal = wait(&status);
				}
			}
		}
		
		//Close the read and write in the parent 
		close(read);
		close(write);
		//Frees path
		free(argvpath);

	//EOF While loop
	}	
	
	//JFree is, argvpath
	jettison_inputstruct(is);
	return 0;
}
