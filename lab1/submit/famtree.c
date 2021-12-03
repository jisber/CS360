//Jacob Isber September 6th, 2020, Lab 1
//In this program we take a list of family and children and then pipe them into a tree and sort them 
/*This program was a bit weird due to being in C but wasn't much much different. 
The TA Philip helped me alot with the children and sex function. He also help me set up my struct*/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fields.h"
#include "dllist.h"
#include "jrb.h"

//Person struct for the program
typedef struct Person
{
    char* Name;
    char* Sex;
    struct Person *Father;
    struct Person *Mother;
    Dllist Children;
    int visited; 
    int printed; 
    
}Person;

//All Function Prototypes
char *name_create(IS is, char* name);
Person *person_create(char *name);
int is_descendant(Person *new_person);
void printList(Person *person);
int setGender(char* gender, Person* p);
int is_descendant(Person *new_person);
void main_print(JRB tree);
void helpPrint(Person *main_person, Dllist toprint);

//Start of main
int main(int argc, char** argv)
{   
    Person *new_person, *child, *person;
    int i = 0;
    Dllist dnode = new_dllist();
    JRB treetest;
    char* name;
    
    //Took straight from lecture notes, "jrb.h".
    JRB peopletree = make_jrb();

    //Took straight from Plank lecture notes "fields.h". 
    IS is; 
    is = new_inputstruct(NULL);

   while(get_line(is) >= 0) 
   {
       if(is -> NF > 1) 
       {
           //Tag line is person, create a name
             if(strcmp(is->fields[i], "PERSON") == 0) 
             {
                name = name_create(is, name);
                if(jrb_find_str(peopletree, name) == NULL)//Checks if name is in tree
                {
                    new_person = person_create(name); //Creates person
                    jrb_insert_str(peopletree, new_person->Name, new_jval_v((void *) new_person));//Inserts into the tree
                }
                    treetest = jrb_find_str(peopletree, name);//Sets the temp tree to the person. 
                    new_person = (Person* ) treetest ->val.v;//If the person is already in the tree set new_person to main person
             }

             if(strcmp(is->fields[0], "FATHER") == 0)//Checks for father
             {
                 name = name_create(is, name);//Creates the name
                 treetest = jrb_find_str(peopletree, name);//temp tree
                 if(treetest == NULL)//If name is not found in the tree 
                 {
                    person = person_create(name);//Creates a person with name
                    person->Sex = "Male";//Set the sex
                    jrb_insert_str(peopletree, person->Name, new_jval_v((void *) person));//Inserts into tree
                 }
                 else //If new_person was already found
                 {
                    if(new_person->Father == NULL)//Checks if the father is set
                    {
                        person = (Person *) treetest->val.v;
                        if(strcmp(person->Sex, "Unknown") != 0)//Checks sex
                        {
                            if(strcmp("Male", person->Sex) != 0) 
                             {  
                                 fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);   //Error if there is a problem with sex mismatch 
                                 exit(1);
                            }
                        }
                        else                       
                            person->Sex = "Male";
                                               
                    }
                    else if(strcmp(new_person->Father->Name, name) != 0)
                    {
                        fprintf(stderr, "Bad input -- child with two fathers on line %d\n", is->line);
					    return 1;
                    }

                 }
                 if(new_person->Father == NULL)//If father is null          
                    new_person->Father = person; //set the father
                 
                 dll_append(person->Children, new_jval_v(new_person));//Insets into the list
             }
            //Exact same mother as father. Please refere to father comments for futher details.      
             if(strcmp(is->fields[0], "MOTHER") == 0)
             {
                 name = name_create(is, name);
                 treetest = jrb_find_str(peopletree, name);
                 if(treetest == NULL)
                 {
                     person = person_create(name);//assigns new_person with those fields
                     person->Sex = "Female";
                     jrb_insert_str(peopletree, person->Name, new_jval_v((void *) person));//Inserts into tree
                 }
                 else 
                 {
                     if(new_person->Mother == NULL)
                     {
                        person = (Person *) treetest->val.v;
                        if(strcmp(person->Sex, "Unknown") != 0) 
                        {
                            if(strcmp("Female", person->Sex) != 0)
                            {  
                                fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);   
                                exit(1);
                            }
                        }
                        else                        
                            person->Sex = "Female";
                        
                     }
                       else if(strcmp(new_person->Mother->Name, name) != 0)
                    {
                        fprintf(stderr, "Bad input -- child with two mothers on line %d\n", is->line);
					    return 1;
                    }
                 }
                 if(new_person->Mother == NULL)               
                    new_person->Mother = person;
                 
                 dll_append(person->Children, new_jval_v((void *) new_person));  
             }
            //Exact same method as father with slight changes. Please refere to father for more detail 
             if(strcmp(is->fields[0], "FATHER_OF") == 0)
             {
                name = name_create(is, name);
               
                 new_person->Sex = "Male";
                 treetest = jrb_find_str(peopletree, name);
                 if(treetest == NULL)
                 {
                    person = person_create(name);//assigns new_person with those fields                  
                    jrb_insert_str(peopletree, person->Name, new_jval_v((void *) person));//Inserts into tree
                 }
                 else 
                    person = (Person *) treetest->val.v;
                 
                  if(person->Father == NULL)
                    person->Father = new_person;
                 
                dll_append(new_person->Children, new_jval_v((void *) person));                
             }
            //Exact same method as father with slight changes. Please refere to father for more detail 
             if(strcmp(is->fields[0], "MOTHER_OF") == 0) //New_Person (Main), Person
             {
                name = name_create(is, name);
                treetest = jrb_find_str(peopletree, name);

                 new_person->Sex = "Female";
                 if(treetest == NULL)
                 {
                    person = person_create(name);//assigns new_person with those fields
                    jrb_insert_str(peopletree, person->Name, new_jval_v((void *) person));//Inserts into tree
                 }
                 else 
                    person = (Person *) treetest->val.v;
                

                 if(person->Mother == NULL)
                    person->Mother = new_person;
                 
                dll_append(new_person->Children, new_jval_v(person));
                    
             }  
             //Checks the sex
              if(strcmp(is->fields[0], "SEX") == 0)
              {    
                     
                if(strcmp(new_person->Sex, "Unknown") != 0)//If the persons sex is not unkown
                {
                    if(strcmp(new_person->Sex,"Male") == 0 && strcmp(is->fields[1], "F") == 0)//Checks for sex mismatch 
                    {
                        fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);   
                        exit(1);
                    }
                    else if(strcmp(new_person->Sex, "Female") == 0 && strcmp(is->fields[1], "M") == 0)//Checks for sex mismatch
                    {
                        fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);   
                        exit(1);
                    }
                }
                else 
                {          
                     if(strcmp(is->fields[1], "M") == 0)//If there is no sex mismatch set the sex 
                    {
                        new_person->Sex = "Male";
                    }
                    else new_person->Sex = "Female";
                }
                
              }    
       }
       
   }
    treetest = jrb_first(peopletree);
    person = (Person *) treetest->val.v;

    //Checks for cycles 
    if(is_descendant(person) == 1)
    {
        fprintf(stderr, "Bad input -- cycle in specification\n");
        return 0;
    }

    //Prints everything 
    main_print(peopletree);
    
    return 0;
}

//Creates the name
char *name_create(IS is, char *name) {
    int i, nsize;
    nsize = strlen(is -> fields[1]);
    for(i = 2; i < is -> NF; i++){
        nsize += (strlen(is -> fields[i])+1);
    }

    name = malloc(sizeof(char) * nsize+ 1);
    strcpy(name, is->fields[1]);
//Concats the name together
    for(i=2; i< is-> NF; i++){
        strcat(name, " ");
        strcat(name, is -> fields[i]);    
    }
    return name;
}

//Creates the person, Mallocs, and sets the fields. 
Person *person_create(char *name) {
	Person *new_person; 
    new_person = malloc(sizeof(Person));
	new_person->Name = strdup(name);
	new_person->Sex = malloc(8);
	strcpy(new_person->Sex, "Unknown");
	new_person->Children = new_dllist();

	new_person->visited = 0;
    new_person->printed = 0;
	
	return new_person;
}

//Checks for cycles this was taken from plank
int is_descendant(Person *new_person) {
	Person *test_person;
	Dllist jrbtest;

	if (new_person->visited == 1) return 0;
									
	if (new_person->visited == 2) return 1;  

	new_person->visited = 2;

	dll_traverse(jrbtest, new_person->Children) 
    {
		test_person = (Person *) jrbtest->val.v;
		if(is_descendant(test_person)) return 1;
	}

	new_person->visited = 1;

	return 0;
}

//Checks the gender 
int setGender(char* gender, Person* person)
{
    if(person->Sex != "Unknown") {
        if(strcmp(person->Sex,gender) != 0)
            return 0;
    }
    return 1;
}

//Print function the just prints the list
void printList(Person* new_person){
    Dllist list;
    Person* new_child;
    if(dll_empty(new_person->Children) != 1){
        printf("\n");
        dll_traverse(list, new_person->Children){
            new_child = (Person*) list->val.v;
            printf("    %s\n", new_child->Name);
        }
    }
}

//Help print function that just prints and adds children to the list
void helpPrint(Person *main_person, Dllist toprint)
{   
    Person *person;
    Dllist temp;

    printf("%s\n", main_person->Name);
    printf("  Sex: ");
    if(main_person->Sex == "NULL")
        printf("Unknown\n");
    else printf("%s\n", main_person->Sex);

    printf("  Father: ");
    if(main_person->Father == NULL)
        printf("Unknown\n");
    else 
    {
    person = (Person *) main_person->Father;
        printf("%s\n", person->Name);
    }
    
    printf("  Mother: ");
    if(main_person->Mother == NULL)
        printf("Unknown\n");
    else{
        person = (Person *) main_person->Mother;
        printf("%s\n", person->Name);
    }
    
        printf("  Children: ");
        person = (Person*) main_person->Children->flink->val.v;
        if(person == NULL)
        printf("None\n");
    else
        printList(main_person);

    printf("\n\n");
    //Traverase and adds the children to toprint
    dll_traverse(temp, main_person->Children){
        person = (Person*) temp->val.v;
        dll_append(toprint, new_jval_v(person));
    }
    //Sets printed to 1 
    main_person->printed = 1;
}

//Main print funciton 
void main_print(JRB peopletree)
{
    Dllist toprint, temp;
    JRB new_tree;
    Person *main_person, *person;
    toprint = new_dllist();
   //Adds everyone to toprint
   jrb_traverse(new_tree, peopletree)
   {
       main_person = (Person *) new_tree->val.v;
       dll_append(toprint, new_jval_v(main_person));
   }
    //Checks for cycles 
   dll_traverse(temp, toprint)
   {
       person = (Person *) temp->val.v;
       if(is_descendant(person))
       {
            fprintf(stderr, "Bad input -- cycle in specification\n");
            exit(-1);
       }
   }
//Start of while for print
   while(dll_empty(toprint) != 1) 
   {
        temp = dll_first(toprint);
        main_person = (Person*) temp->val.v;
        dll_delete_node(temp);
        if(main_person->printed == 0)//If the main person has not been printed 
        {
            if(main_person->Father == NULL) //checks if the father has been set 
            {
                if(main_person->Mother == NULL)//Checks if the mother has been set
                {
                    helpPrint(main_person, toprint);//Calls help print function to print and add children to the list
                
                }
                else if(main_person->Mother->printed == 1)//If mother has been printed 
                {
                    helpPrint(main_person, toprint);//Calls help print function to print and add children to the list
                }
         
            }
            else if(main_person->Mother == NULL)//If mother has been set 
            {
                if(main_person->Father->printed == 1)//If father has been prnted 
                {
                    helpPrint(main_person, toprint);//Calls help print function to print and add children to the list
                }
            }
            else if(main_person->Mother->printed == 1 && main_person->Father->printed == 1)//If mother and father have been printed 
            {
                helpPrint(main_person, toprint);//Calls help print function to print and add children to the list
            }
        }
   }
}
