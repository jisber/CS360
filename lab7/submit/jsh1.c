#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "fields.h"
#include "jrb.h"

void cntl_d_handler(int dummy)
{
	printf("You just typed cntl-d\n");
	signal(SIGINT, cntl_d_handler);
	exit(1);
}

int main(int argc, char ** argv)
{

	int i, status, ampTrigger = 0, id, waitVal, count = 0;
	IS is = new_inputstruct(NULL);
	char *path;
	char **argvpath;
	char *argv1 = argv[1];

	if(argc > 2) 
	{ 
		fprintf(stderr, "usage: jsh1 [prompt]\n");
		exit(1);
	}

	if(argv[1]==NULL)  
		printf("jsh: "); 

	else if(strcmp(argv1, "-")!=0) 
		printf("%s: ", argv1); 

	signal(SIGINT, cntl_d_handler);

	while(get_line(is) >= 0)
	{
		count = 0;

		if(argv[1]==NULL)  
			printf("jsh: ");  

		else if(strcmp(argv1, "-")!=0) 
			printf("%s: ", argv1);

		ampTrigger = 0;  
        
		if(is->NF > 0)
		{
			path = is->fields[0];
			argvpath = (char**) malloc(sizeof(char*)*is->NF + 1);
			for(i = 0; i < is->NF; i++)
			{ 	
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
		}
		argvpath[count] = NULL;
		id = fork();
		if(id==0)
		{
			execvp(argvpath[0], argvpath);
			perror(argvpath[0]);
			exit(1);
		}
		else if(ampTrigger==0)
		{ 
			waitVal = wait(&status);
			while(waitVal != id)
			{
				waitVal = wait(&status);
			}
		}

	//end of while
	}	

	jettison_inputstruct(is);
	return 0;
}