//Jacob Isber, Lab3 FakeMake. 
//In this lab we essentially made a fake make file which just compiled a file. It was fairly easy and freeing up the memory is getting alot easier. 
//Beta 1.4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fields.h"
#include "dllist.h"

//Function Prototypes
int maxTime(Dllist, struct stat, int, int);
char* catList(Dllist list, char* c);

int main(int argc, char **argv) 
{
  
  IS is;
  Dllist c_list, h_list, l_list, e_list, f_list, o_list, temp_list1, temp_list2;
  struct stat buffer, buffer_o;
  int i, max_h_time, max_o_time, is_compiled, c_length, trigger = 0, h_input, c_input, o_input; 

  //Error check for too many arguments
  if(argc > 2) 
  {
    fprintf(stderr, "error: too many arguments\n");
	exit(1);
  }

  //Error check for if the slot at argv[1] is empty
  if(argv[1] != NULL) 
  {
    is = new_inputstruct(argv[1]);
    if(is == NULL) 
    {
      fprintf(stderr, "error: file does not exists.\n");
      exit(1);
	  }
  }
  else
	  is = new_inputstruct("fmakefile");

  //Creats instances of new Dlists 
  c_list = new_dllist();
  h_list = new_dllist();
  l_list = new_dllist();
  e_list = new_dllist();
  f_list = new_dllist();
  o_list = new_dllist();

  trigger = 0;
  //Reads through is
  while(get_line(is) != -1) 
  {
    
    //Lines 55 - 86 simply checks the starting char at each new line and reads it into a Dlist
	  if(strcmp(is->fields[0], "C") == 0)
	    for(i = 1; i < is -> NF; i++)
	      dll_append(c_list, new_jval_s(strdup(is->fields[i])));


	  else if(strcmp(is -> fields[0], "H") == 0)
	    for(i = 1; i < is -> NF; i++)
	      dll_append(h_list, new_jval_s(strdup(is->fields[i])));


	  else if(strcmp(is -> fields[0], "L") == 0)
	    for(i = 1; i < is -> NF; i++)
	      dll_append(l_list, new_jval_s(strdup(is -> fields[i])));

    //E can only hold a single item. So if the trigger is set that E is filled with more than 1 thing print an error.
	  else if(strcmp(is -> fields[0], "E") == 0) 
    {
	    if(trigger == 1) 
      {
        fprintf(stderr, "fmakefile (%d) cannot have more than one E line\n", is->line);
        exit(1);
	    }
	    else 
      {
        for(i = 1; i < is -> NF; i++) 
        {
          dll_append(e_list, new_jval_s(strdup(is->fields[i])));
          trigger = 1;
        }
      }
	  }

//Reads in for the F line
	  else if(strcmp(is->fields[0], "F") == 0)
	    for(i = 1; i < is -> NF; i++)
	      dll_append(f_list, new_jval_s(strdup(is->fields[i])));

  //End of while()
  }

//If the trigger is 0, it means the E Dlist is empty therefore no executable was given. 
  if(trigger == 0) {
	fprintf(stderr,"No executable specified\n");
    exit(1);
  }	

 
  //Grabs max time for the header file. 
  max_h_time = 0;
  max_h_time = maxTime(h_list, buffer, max_h_time, h_input);

  is_compiled = 0;
  int command;

  //Traverses through the c list and compiles. 
  dll_traverse(temp_list1, c_list) 
  {
      c_input = stat(jval_s(temp_list1 -> val), &buffer);

    if(c_input < 0) {
      fprintf(stderr, "fmakefile: %s: No such file or directory\n", jval_s(temp_list1->val));
      exit(1);
    }

    else 
    {
      //Sets the o flag
      char comp_line_arr[256] = "gcc -c";
      c_length = strlen(jval_s(temp_list1->val));
      char *compiled_o = strdup(jval_s(temp_list1->val));
      strcpy(compiled_o+(c_length-1), "o");
      dll_append(o_list, new_jval_s(strdup(compiled_o)));  

      o_input = stat(compiled_o, &buffer_o);

      free(compiled_o);
	//Compaes the times to see if anything has changed. 
      if(o_input != 0 || buffer_o.st_mtime <= buffer.st_mtime || buffer.st_mtime <= max_h_time) 
      {
        dll_traverse(temp_list2, f_list) 
        {
          strcat(comp_line_arr, " ");
          strcat(comp_line_arr, jval_s(temp_list2->val));
        }

        strcat(comp_line_arr, " ");
        strcat(comp_line_arr, jval_s(temp_list1->val));

        printf("%s\n", comp_line_arr);

        command = system(comp_line_arr);
        if(command != 0) 
        {
          fprintf(stderr, "Command failed.  Exiting\n");
          exit(1);
        } 
        is_compiled = 1;
      }
    }
  }
  
  //Max run time for o files
  max_o_time = 0;
  max_o_time = maxTime(o_list, buffer_o, max_o_time, o_input);
  
  char compile_code[256] = "gcc -o";
  
  //Traverses the E list and outputs everything. 
  dll_traverse(temp_list1, e_list) {
    int e_check = stat(jval_s(temp_list1->val), &buffer);

    if(is_compiled > 0 || buffer.st_mtime < max_o_time) 
    {
      strcat(compile_code, " ");
      strcat(compile_code, jval_s(temp_list1->val));
	
	//Calls catList to concat everything 
      catList(f_list, compile_code);
      catList(o_list, compile_code);
      catList(l_list, compile_code);
        
      printf("%s\n", compile_code);

      command = system(compile_code);
      
      if(command != 0) 
      {
        fprintf(stderr, "Command failed.  Fakemake exiting\n");
        exit(1);
      }	  
	}
  else 
  {
    printf("%s up to date\n", jval_s(temp_list1->val));
    exit(1);
  }
}

  //Frees everything inside of each list
  dll_traverse(temp_list1, c_list)
	free(jval_s(temp_list1->val));

  dll_traverse(temp_list1, h_list)
	free(jval_s(temp_list1->val));

  dll_traverse(temp_list1, l_list)
	free(jval_s(temp_list1->val));

  dll_traverse(temp_list1, e_list)
	free(jval_s(temp_list1->val));

  dll_traverse(temp_list1, f_list)
	free(jval_s(temp_list1->val));

  dll_traverse(temp_list1, o_list)
	free(jval_s(temp_list1->val));

  //Frees the list 
  free_dllist(c_list);
  free_dllist(h_list);
  free_dllist(o_list);
  free_dllist(l_list);
  free_dllist(e_list);
  free_dllist(f_list);

  //Frees the is
  jettison_inputstruct(is);


  return 0;
}

//Concats what ever is a Dlist
char* catList(Dllist list, char* c)
{ 
  Dllist test2;

  dll_traverse(test2, list) 
  {
    strcat(c, " ");
    strcat(c, jval_s(test2->val));
  }
  return c;
}

//Returns the max time of a compile time. 
int maxTime(Dllist list, struct stat buffer , int max_time, int in)
{
  Dllist temp_list;
  max_time = 0;
  dll_traverse(temp_list, list) {	  
	in = stat(jval_s(temp_list->val), &buffer);
  if(in < 0)
  {
	  fprintf(stderr, "Error\n", jval_s(temp_list->val));
	  exit(1);
	}
  else
	  if(buffer.st_mtime > max_time)
		  max_time = buffer.st_mtime;
  }

  return max_time;
}
