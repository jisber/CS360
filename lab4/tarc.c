//Jacob Isber, lab4. Tarc. 
//The bulk of this program was taken from Master Plank's prize8.c. This programs takes a tar file and compresses it. 
//Beta 1.3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "jrb.h"
#include <sys/types.h>
#include "fields.h"
#include "dllist.h"
#include "jval.h"

//Function prototypes
void get_size(JRB inodes, char *fn, int offset);
int shortName_Offset(char *name);

int main(int argc, char **argv)
{

    struct stat buf;
    int exists, name_length;
    DIR *dir;
    struct dirent *de;
    JRB inodes, tmp;
    char *fn;
    char directory_name[100], temp_name[100];
    char *tmp_offset;
    int offset = 0;

    //creates inode tree
    inodes = make_jrb();

    //opens directory
    dir = opendir(".");

    //Error check for argc
    if(argc != 2){
        fprintf(stderr, "usage: tarc directory\n");
        exit(1);
    }

    exists = lstat(argv[1], &buf);

    strcpy(directory_name, argv[1]);
    strcpy(temp_name, argv[1]);
    fn = strdup(argv[1]);

    //Grabs the offset
    offset = shortName_Offset(directory_name);

    //Sets ther namelenghts offset
    tmp_offset = temp_name + offset;
    name_length = (int) strlen(tmp_offset);

    //Writes the the files the necessary info
    fwrite(&name_length, sizeof(int), 1, stdout);  
    printf("%s", tmp_offset);
    fwrite(&buf.st_ino, sizeof(buf.st_ino), 1, stdout);
    fwrite(&buf.st_mode, sizeof(buf.st_mode), 1, stdout);    
    fwrite(&buf.st_mtime, sizeof(buf.st_mtime), 1, stdout);

    //Gets the sizes, closes the directories, traverses and frees the vals. frees the tree, frees the file
    get_size(inodes, fn, offset);
    closedir(dir);    
    jrb_traverse(tmp, inodes)
    {
        free(tmp->val.v);
    }
    jrb_free_tree(inodes);
    free(fn);

    return 0;
}

//Function to grab the offset. 
int shortName_Offset(char *name)
{
    char *end;
    int offset = 0;

    end = strrchr(name, '/');
    offset = (int)(end - name + 1);

    if(offset > 300 || offset <= 0) return 0;

    return offset;
}


void get_size(JRB inodes, char *fn, int offset){

    DIR *dir;
    struct dirent *de;
    struct stat buf;
    int exists;
    char *s;
    Dllist directories, tmp;
    directories = new_dllist();

    //Opens the directory
    dir = opendir(fn);
    if (dir == NULL) {
        perror("Unable to open directory");
        exit(1); 
    }

    s = (char *) malloc(sizeof(char)*(strlen(fn)+258));

    //Reads in the file name
    for (de = readdir(dir); de != NULL; de = readdir(dir)) {
        sprintf(s, "%s/%s", fn, de -> d_name);
        exists = lstat(s, &buf);
        if (exists < 0) {
            fprintf(stderr, "Couldn't stat %s\n", s);
            exit(1);
        }

        //Checks if there is a period or a double period. if there is continue. 
        if(strcmp(de->d_name, ".") == 0) continue;
        else if (strcmp(de->d_name, "..") == 0) continue;

        else{
            //Sets the offset
            char *tmp;
            tmp = s + offset;
            int fn_len;
            fn_len = (int) strlen(tmp);  

            fwrite(&fn_len, sizeof(int), 1, stdout);
            printf("%s", s + offset);       
            fwrite(&buf.st_ino, sizeof(buf.st_ino), 1, stdout);
        
        //looks if the inode is not in the tree, if not insert it. 
         if (jrb_find_int(inodes, buf.st_ino) == NULL){
             jrb_insert_int(inodes, buf.st_ino, new_jval_i(0));
                
            fwrite(&buf.st_mode, sizeof(buf.st_mode), 1, stdout);   
            fwrite(&buf.st_mtime, sizeof(buf.st_mtime), 1, stdout);

            //Checks if the file is a directory or not
            if (S_ISDIR(buf.st_mode)) {
                dll_append(directories, new_jval_s(strdup(s)));
            }
            //If it is just a file
            else
            {
                fwrite(&buf.st_size, sizeof(buf.st_size), 1, stdout);
                FILE *f;
                f = fopen(s, "r");
                char buffer[buf.st_size];
                fread(buffer, buf.st_size, 1, f);
                fwrite(&buffer, buf.st_size, 1, stdout);
                fclose(f);
            }

            }
        }
    }    

    //Closes directory
    closedir(dir);

    //Traverses through the directories list and frees
    dll_traverse(tmp, directories) {
        get_size(inodes, tmp->val.s, offset);
        free(tmp->val.s);   
    }

    //Frees directories
    free_dllist(directories);
    //Frees s
    free(s);
}
