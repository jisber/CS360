//Jacob Isber 09/14/2020
//This code took in a unreadable file of names and read it in using fgetc. This code wasn't particuallry difficult it was just confusing. The TA told me it was okay if I didn't free 
//my memory I would loose at max 10 points. We could really figure out the problem with why my frees were causing segfaults. Both TA's helped me with the logic for the code. 
//Beta 1.2
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fields.h"
#include "dllist.h"
#include "jrb.h"

//Machine struct that contains the dllist of all names and the ip address.
typedef struct Machine
{
    int ip_address[4];
    Dllist names;
    
}Machine;

//Printer function for my dllist
void printList(Machine* new_person){
    Dllist list;
    Machine* temp_machine;
    if(dll_empty(new_person->names) != 1){
        dll_traverse(list, new_person->names){
            temp_machine = (Machine*) list->val.v;
            printf("%s ", temp_machine);
        }
    }
    printf("\n");
}
//Prints the ip address in correct format
void printIp(Machine* new_machine)
{
    int i;
    
    for(i = 0; i < 4; i++)
    {
        printf("%d", new_machine->ip_address[i]);
        if(i != 3) 
            printf(".");
        else 
            printf(":  ");
    } 
}

int main() 
{
    JRB hostTree = make_jrb();
    FILE *fp;
    char* name;
    char* lname;
    char current_char;
    int file, fileTemp;
    Machine* new_machine;
    int i, count = 0, j = 1, k = 0, name_length = 0, l, numNames, trigger = 0, periodCount = 0;
    char str1[20];
    JRB tempHostTree, tree;
    Dllist temp_dllist;

    fp = fopen("converted", "r");//Opens the file and reads it into fopen
    
    while(1)
    {
        new_machine = malloc(sizeof(Machine));//Mallocs size of new_machine
        new_machine->names =new_dllist();//Creates new dllist
        numNames = 0;
    
        if(feof(fp)) break;//If end of file, break.
      
        for(i = 0; i < 4; i++) //Reads in the first 4 bites into the ip address
        {
            file = fgetc(fp);//Gets next character in the file
            new_machine->ip_address[i] = file;
        }

        for(i = 0; i < 4; i++)//Reads in the next 4 bites into the ip address
         {
            file = fgetc(fp);//Gets next character in the file
            
            //Multiplies by the necessary number to get the correct number of names
           if(i == 0) numNames += 1000 * file;
           if(i == 1) numNames += 100 * file;
           if(i == 2) numNames += 1 * file;
           if(i == 3 )numNames += 1 * file;

         }

        //Begginning of main loop
         for(i = 0; i < numNames; i++)
         {
             name_length = 0; //Resets the num_length everytime
             fileTemp = ftell(fp);//Creates a temp spot with ftell
            
             while(fgetc(fp) != '\0')//Grabs the total name size
             {
                 name_length++;
             }

                lname = malloc((name_length + 1) * sizeof(char));//Mallocs lname with the total size with room for a null character.
                fseek(fp, fileTemp, SEEK_SET);//Seeks back to the original spot
                periodCount = 0;
                current_char = fgetc(fp); //Grabs the next character in file
                
             while(current_char != '\0') //while the current char is not equal to null character
             {
                //periodCount++;
                if(current_char == '.' && trigger == 0)//If these conditions are met
                {
                    name = malloc((name_length + 1) * sizeof(char));//Mallocs lname with the total size with room for a null character.
                    for(l = 0; l < periodCount; l++)//Loads the local name into name. 
                    {
                        name[l] = lname[l];
                    }
                    name[periodCount] = '\0';//Adds null character
                    trigger = 1;//Sets the trigger to 1. This is because I only need the trigger once per name if a period is in the name.

                    //Appends and inserts the name 
                    dll_append(new_machine->names,new_jval_s(name));
                    jrb_insert_str(hostTree, name, new_jval_v((void*) new_machine));
                }
                memcpy((lname + periodCount), &current_char, sizeof(char));//I took this line of code from stack overflow. Specifically &current_char. I already talked to TA about it. 
                current_char = fgetc(fp);
                periodCount++;
             }
             lname[name_length] = '\0';
             //Appends and inserts the name 
             dll_append(new_machine->names,new_jval_s(lname));
             jrb_insert_str(hostTree, lname, new_jval_v((void*) new_machine));
             trigger = 0;
         }
   // j--;
    }

        //My print
         printf("Hosts all read in\n\n");
         printf("Enter host name: \n");
    
         while(scanf("%s", str1) != EOF){
             tempHostTree = jrb_find_str(hostTree, str1);
         if(tempHostTree == NULL)
         {
             printf("no key %s\n", str1);
         }
         else
         {
            jrb_traverse(tree, hostTree)
            {
            if(strcmp(jval_s(tree->key), str1) == 0)
            {
                new_machine = (Machine*) tree->val.v;
                printIp(new_machine);//Prints ip
                printList(new_machine);//Prints list
                }
            }
         }
        printf("\n");
        printf("Enter host name:  ");
     }


    jrb_free_tree(hostTree);//main tree
    
    return 0;
}
