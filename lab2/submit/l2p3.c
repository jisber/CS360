//Jacob Isber 09/14/2020
//This code took in a unreadable file of names and read it in using open() and using a buffer and a main_count. This code wasn't particuallry difficult it was just confusing.
//The TA told me it was okay if I didn't free my memory I would loose at max 10 points. We could really figure out the problem with why my frees were causing segfaults. 
//Both TA's helped me with the logic for the code.
//Beta 1.2
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "fields.h"
#include "dllist.h"
#include "jrb.h"

//My machine struct
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

//Prints ip address into correct format
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
    JRB hostTreeMain = make_jrb();
    char* name;
    int fp;
    char* lname;
    char current_char;
    int file, fileTemp;
    Machine* new_machine;
    int i, count = 0, j =  13000, k = 0, name_length = 0, l, main_count = 0, numNames, trigger = 0, periodCount = 0;
    char str1[20];
    unsigned char buffer[350000];
    JRB tempHostTree, tree;
    Dllist temp_dllist;

    fp = open("converted", O_RDONLY);//Opens the file in read only
    read(fp, buffer, 350000);//Reads in the file to the buffer
    
    while(j != 0)//main_count < 350000
    {
        new_machine = malloc(sizeof(Machine));//Mallocs size of new_machine
        new_machine->names =new_dllist();//Creates new dllist
        numNames = 0;
        
        //Loads the bites from the buffer into the ip_address
        for(i = 0; i < 4; i++)
        {  
            new_machine->ip_address[i] = buffer[main_count];
            main_count++;//Counts the main_count
        }

        for(i = 0; i < 4; i++)
         {  
            unsigned int temp_int = buffer[main_count];//Converts buffer[main_count] to an int

           if(i == 0) numNames += 1000 * temp_int;
           if(i == 1) numNames += 100 * temp_int;
           if(i == 2) numNames += 1 * temp_int;
           if(i == 3 )numNames += 1 * temp_int;

            main_count++; //Increments the main_count
         }
        
        //Begginning of main loop
         for(i = 0; i < numNames; i++)
         {
             name_length = 0;
             while(buffer[main_count] != '\0')//Grabs the total name length of the first name
             {
                 name_length++;
                 main_count++;
             }
                lname = malloc((name_length + 1) * sizeof(char));//Mallocs lname with the total size with room for a null character.
                main_count = main_count - name_length;//Resets the main_count counter to the beginnging of the name
                periodCount = 0;//Sets period count
                current_char = buffer[main_count];//Grabs the carracter at main_count

             while(current_char != '\0')//While the current character isn't null
             {
                //periodCount++;
                if(current_char == '.' && trigger == 0)//If these conditions are met
                {  
                    name = malloc((name_length + 1) * sizeof(char));//Mallocs lname with the total size with room for a null character.
                    for(l = 0; l < periodCount; l++)//Loads the lname wich is the name up until the first period. 
                    {
                        name[l] = lname[l];
                    }
                    name[periodCount] = '\0';//Adds the null char to the end of name
                    trigger = 1;//Sets the trigger to 1. This is because I only need the trigger once per name if a period is in the name.
                    
                    //Appends and inserts the name 
                    dll_append(new_machine->names,new_jval_s(name));
                    jrb_insert_str(hostTreeMain, name, new_jval_v((void*) new_machine));
                }

                memcpy((lname + periodCount), &current_char, sizeof(char));//I took this line of code from stack overflow. Specifically &current_char. I already talked to TA about it.

                main_count++;//Increments the main_count
                current_char = buffer[main_count];//Sets the current char to the next char in buffer
                periodCount++;
             }
             lname[name_length] = '\0';//Adds the null char to the list. 

             //Append the names into the list and insert into the tree. 
             dll_append(new_machine->names,new_jval_s(lname));
             jrb_insert_str(hostTreeMain, lname, new_jval_v((void*) new_machine));
             trigger = 0;//Resets the trigger
             main_count++;//Increments the main counter
         }
     j--;
    }
  
  //This is my print for the whole program 
         printf("Hosts all read in\n");
         printf("\n");
         printf("Enter host name: \n");
    
         while(scanf("%s", str1) != EOF){
             tempHostTree = jrb_find_str(hostTreeMain, str1);
         if(tempHostTree == NULL)
         {
             printf("no key %s\n", str1);
         }
         else
         {
            jrb_traverse(tree, hostTreeMain)
            {
            if(strcmp(jval_s(tree->key), str1) == 0)
            {
                new_machine = (Machine*) tree->val.v;
                printIp(new_machine); //Prints ip in corrct format
                printList(new_machine);//Prints the list. 
                }
            }
         }
        printf("\n");
        printf("Enter host name:  ");
     }

    jrb_free_tree(hostTreeMain);        
    return 0;
}