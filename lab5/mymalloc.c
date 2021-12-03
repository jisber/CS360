//Jacob Isber Lab5 Jmalloc
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mymalloc.h"

//Given struct
typedef struct flist {

   int size;     
   struct flist *flink; 
   struct flist *blink; 
} *Flist;

//Global variable, pointer to the head of the list
void *malloc_head = NULL;


void *my_malloc(size_t size)
{
  
  int *ptr;
  Flist memory, buf;
  
  //Sets the size
  size = (size + 15 ) & -8;

  //Checks if the head of the list is null (empty)
  if(free_list_begin() == NULL)
  {
    //Check
    if(size <= 8192)
    {
      //Allocate memoery, set a memory node = to the head, set the size and set the next node to NULL, chunkates and returns the size of the memory address
      malloc_head = sbrk(8192);
      memory =  malloc_head;
      memory -> size = 8192 - (size);
      memory -> flink = NULL;
      ptr = (int *)(malloc_head + memory -> size);
      *ptr = size;
      return (void *)ptr + 8;
    }
    //If the size is bigger just allocate and return that memory
    else
    {
      memory = (Flist) sbrk(size);
      memory->size = size;
      memory = (void *) memory + 8;
      return  memory;
    }
  }
  //If list is not empty 
  else
  {
    memory = malloc_head;
    buf = NULL;
    //Loops through the nodes in the list and runs a check
    for(memory = free_list_begin(); memory != NULL; memory = (Flist)free_list_next(memory)) {

      if(memory->size >= size)
      {
        buf = memory;
        break;
      }
    }
    //if buf is not null we found a memory chunk, and check to see if it is greater that 15 beacuse of padding
    if(buf != NULL)
    { 
      if((buf -> size - size) >= 15)
      {
        buf -> size = buf->size - size;
        ptr = (int*)((void*)buf + buf -> size);
        *ptr = size;
        return (void *)ptr + 8;
      }
      //Set the node and return the address
      else
      {
        Flist node;
        node = buf;
        if(buf == malloc_head) malloc_head = buf -> flink;
        return (void*)node + 8;
      }
    }
    //if there isn't enough memory
    else
    { 
    //Allocate more mem
      if(size > 8192)
      {
        memory = (Flist) sbrk(size);
        memory->size = size;
        memory = (void *) memory + 8;
        return  memory;
      }
      else
      {
        Flist prev_node;
        prev_node = malloc_head;
        malloc_head = sbrk(8192);
        memory = malloc_head;
        memory -> flink = prev_node;
        memory -> size = 8192 - (size);
        ptr = (int *)(malloc_head + memory -> size);
        *ptr = size;
        return (void *)ptr + 8; 
      }
    }
  }
}

//Frees my nodes
void my_free(void *ptr)
{
  Flist buf;
  buf = malloc_head;
  malloc_head = (Flist) ((unsigned long)(ptr - 8));
  ((Flist) malloc_head) -> flink = buf;

  /* *******************Please ignore this block of code, I was trying to figure out a way to re-order my address that way I didn't have have to call qsort, malloc, and or free
  I want to work on it later but currently don't have the time and didn't want to delete the code. It is almost done but there is an issue with it. ************************* */

  // Flist temp, fPtr;
  
  // fPtr = (void*)ptr;
  // // We want head to have the smallest address (list in ascending order)
  // for (temp = free_list_begin(); temp != NULL; temp = (Flist)free_list_next(temp))
  // {
  //   int lastNode = 0;
  //   if(temp = ((Flist)malloc_head)->blink)
  //     lastNode = 1;

  //   if (&fPtr < &temp)
  //   {
  //     fPtr->flink = temp;
  //     if (temp->blink)
  //     {
  //       temp->blink->flink = fPtr;
  //       fPtr->blink = temp->blink;
  //     }
  //     temp->blink = fPtr;

  //     if(temp == malloc_head)
  //     {
  //       malloc_head = fPtr;
  //     }

  //     break;
  //   }

  //   if(lastNode == 1)
  //     break;
  // }
}

//Return the head of the node
void *free_list_begin(){
  return malloc_head;
}

//Grabs the next node in the list
void *free_list_next(void *node){
  Flist buf = node;
  if(buf -> flink == NULL){
    return NULL;
  }
  return buf->flink;
}

//cmpfunc I grabbed off google for Qsort
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

//Coalesces the address
void coalesce_free_list()
{
  Flist node;
  int i,size_of_node, count = 0;
  
  //Grabs number of nodes inside the dlist
  for(node = free_list_begin(); node != NULL; node = (Flist)free_list_next(node))
  {
    count++;
  }
  
  //Mallocs the list with the amount of nodes
  unsigned long *list = (malloc(sizeof(unsigned long) * count));

  //resets the node to malloc_head
  node =  malloc_head;
  
  //Fills the list with the nodes
  for(i = 0; i < count; i++)
  {
    list[i] = (unsigned long) node;
    node = ((Flist) node) ->flink;
  }

  //Calls qsort() on the list
  qsort(list, count, sizeof(unsigned long), cmpfunc);

  //Resets node postions to the beginning of the list
  malloc_head = (Flist) list[0];
  node = (Flist) list[0];

  //Runs through the list 
  for(i = 0; i < count - 1; i++)
  {
    //Grads the size of the node in the list[i]
    size_of_node = ((Flist)list[i])->size;

    //Checks and sees if they are adjacent, if they are add the adjacent node size to the current size, if not set the next node to the list[i + 1] a
    if(list[i + 1] == list[i] + size_of_node)
    {
      node->size += ((Flist)list[i + 1])->size;
    }
    else
    {
      node->flink = (Flist)list[i + 1];
      node = node->flink;
    }
  }
  //sets next node to null
  node->flink = NULL;

  //Frees the list
  free(list);
}