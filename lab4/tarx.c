//Jacob Isber, Lab4 part B. Tarx.
//This lab similar to tarc but instead uncompresses. This followed the same skeleton as tarc.
//Beta 1.3

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "jrb.h"
#include "dllist.h"

//Function prototype
void timeSet(char* someFile, unsigned long mod_time);

int main() 
{
    int filename_size, mode, check_time;
    long inode, mod_time, file_size;
    char *filename;
    char *buffer;
    JRB inode_tree, mode_tree, mod_time_tree, jrbtmp;
    Dllist directories, temp_l;

    //Creates all the trees and Dllist I need. 
    inode_tree = make_jrb();
    mode_tree = make_jrb();
    mod_time_tree = make_jrb();
    directories = new_dllist();

    while(1)
    {   
        //Reads in the file size. 
        fread(&filename_size, 4, 1, stdin);
        if(filename_size <= 0)
        {
            fprintf(stderr, "Error: \n");
            exit(-1);
        }
        
        //Checks for EOF
        if(feof(stdin)) break;

        //Mallocs filename to be filesize + 1 for space for the null character. 
        filename = malloc(filename_size + 1);

        //Reads in the file name
        if(fread(filename, filename_size, 1, stdin) ==0)
        {
            fprintf(stderr, "Error: \n");
            exit(-1);
        }

        //Adds null character
        filename[filename_size] = '\0';
        
        //Reads in the inodes
        if(fread(&inode, 8, 1, stdin)==0)
        {
            fprintf(stderr, "Error: \n");
            exit(-1);
        }
        
        //Checks if the inode in the tree
        if(jrb_find_int(inode_tree, inode) == NULL)
        {   
            //Reads in the mode
            if(fread(&mode, 4, 1, stdin)==0)
            {
                fprintf(stderr, "Error: \n");
                exit(-1);
            }

            //Reads in the modification time
            if(fread(&mod_time, 8, 1, stdin) == 0)
            {
                fprintf(stderr, "Error: \n");
                exit(-1);
            }  

            //Checks if it is not a directory
            if(!S_ISDIR(mode))
            {   
                //Reads in the file size
                fread(&file_size, sizeof(unsigned long), 1, stdin);
                if(file_size <= 0)
                {
                    fprintf(stderr, "Error \n");
                    exit(-1);
                }
                
                //Sets a buffer
                buffer = malloc(file_size + 1);

                //Reads into the buffer
                fread(buffer, sizeof(char), file_size, stdin);
                buffer[file_size] = '\0';
            }
        }
        //Checks for hard links
        if(jrb_find_int(inode_tree, inode) != NULL)
        {
            jrbtmp = jrb_find_int(inode_tree, inode);
            link(jrbtmp->val.s, filename);
        }
        //If it is a directory
        else if(S_ISDIR(mode))
        {   
            //Make the directory and insert mode and modification time into seperatre trees.
            mkdir(filename,488);
            jrb_insert_int(mode_tree, mode, new_jval_s(filename));
            jrb_insert_int(mod_time_tree, mod_time, new_jval_s(filename));
            
        }
        //If it is a file
        else
        {   
            //Create the file, set the permissions write to the file, set the modes of the file, close the file
            FILE *fp;   
            chmod(filename, 448);            
            fp = fopen(filename, "w");
            fwrite(buffer, sizeof(char), file_size, fp);
            chmod(filename, mode);
            fclose(fp);
            timeSet(filename, mod_time);
            free(buffer);
        }

        //insert inodes into the tree
        jrb_insert_int(inode_tree, inode, new_jval_s(filename));

        //Insert the names for the Dllist. This will be used to free. 
        dll_append(directories, new_jval_s(filename));

    }
    //136-144 Traverses through trees sets the mode and sets the modifiaction time. 
    jrb_traverse(jrbtmp, mode_tree) 
    {   
        chmod(jrbtmp->val.s, jrbtmp->key.i);
    }
    jrb_traverse(jrbtmp, mod_time_tree) 
    {   
        timeSet(jrbtmp->val.s, jrbtmp->key.i);
    }
    
    //Lines 146-155 frees everything malloced.
    jrb_free_tree(inode_tree);
    jrb_free_tree(mode_tree);
    jrb_free_tree(mod_time_tree);

    dll_traverse(temp_l, directories)
    {
        free(jval_s(temp_l->val));
    } 
    free_dllist(directories);

    return 0;
}

//Function to set the times. 
void timeSet(char* someFile, unsigned long mod_time)
{
    struct timeval *set_time;
    set_time = malloc(sizeof(struct timeval) * 2);
    set_time[0].tv_sec = time(NULL);
    set_time[0].tv_usec = 0;
    set_time[1].tv_sec = mod_time;
    set_time[1].tv_usec = 0;
    utimes(someFile, set_time);
    free(set_time);
    return;
}