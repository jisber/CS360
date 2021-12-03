//Jacob Isber, Final Project (Lab A) Threaded Chat Server.
//In this lab our task was to create a threaded chat server
//Beta 1.8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "dllist.h"
#include "jrb.h"
#include "sockettome.h"

//Global JRB 
JRB main_room;

//Room Struct
typedef struct
{
	char *name;
	Dllist clients;
	Dllist messages;
	pthread_mutex_t *lock;
	pthread_cond_t *condition;

} Room;

//Client Struct
typedef struct
{
	int fd;
	Room *client_room;
	char *name;
	FILE *out;
	FILE *in;
	Dllist curr_c_list;

} Client;

//Function Prototypes
void *clientThread(void *thread);
void clientHelper(JRB tree, Client *cc);
void *roomThread(void *thread);

int main(int argc, char **argv)
{
	int i, port, serversocket, fd;
	JRB rooms, tmp_room;
	Room *room;
	Client *cc;
	pthread_t clientT, roomT;
	
	//Creates the JRB
	rooms = make_jrb();

	//Error check for number of arguments. 
	if(argc < 3)
	{
		printf("Not enough arguments\n");
		exit(1);
	}

	//Converts port to int
	port = atoi(argv[1]);
	
	//Checks port value
	if (port < 8000) 
	{
		printf("Port must be >= 8000");
		exit(1);
	}

	//Loads the rooms into a JRB tree called rooms. 
	for(i = 0; i < argc - 2; i++)
	{
		room = (Room*) malloc(sizeof(Room));
		room->messages = new_dllist();
		room->clients = new_dllist();
		room->name = argv[i+2];
		jrb_insert_str(rooms, room->name, new_jval_v((void *) room));
	}

	//Mallocs and threads the server by calling roomThread
	jrb_traverse(tmp_room, rooms)
	{
		room = (Room*) tmp_room->val.v;
		room->lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		room->condition = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
		pthread_mutex_init(room->lock, NULL);
		pthread_cond_init(room->condition, NULL);
		pthread_create(&roomT, NULL, roomThread, ((void*) tmp_room->val.v));
	}

	//Grabs the server_socket with the given port
	serversocket = serve_socket(port);

	//Main while loop
	while(1)
	{
		printf("Waiting for client to connect...\n");

		//accepts the connection and sets the client up, while calling clientThread to thread the user. 
		fd = accept_connection(serversocket);
		cc = (Client*) malloc(sizeof(Client));
		cc->fd = fd;
		cc->in = fdopen(fd, "r");
		cc->out = fdopen(fd, "w");
		main_room = rooms;
		pthread_create(&clientT, NULL, clientThread, cc);
	}

	return 0;
}
//Threads Room
void *roomThread(void *thread)
{
	Room *r = (Room *) thread;
	Client* client;
	char* message;
	Dllist dll_messages, dll_client;
	
	while(1)
	{	
		//Calls signal pthread_cond_wait and waits for the connection. 
		pthread_cond_wait(r->condition, r->lock);

		//While the dllist of messasges is not empty
		while(!dll_empty(r->messages))
		{ 		
			//Grabs the first message 
			dll_messages = dll_first(r->messages);

			//Traverses through the clients list and grabs the client and sends the message 
			dll_traverse(dll_client, r->clients)
			{	
				client = (Client *) jval_v(dll_client->val);
				message = jval_s(dll_messages->val);

				fputs(message, client->out);
				fflush(client->out);
			}
			//Frees memory 
			free(dll_messages->val.s);
			dll_delete_node(dll_messages);
		}
	}
}

//Creates the threading for the client. 
void *clientThread(void *thread)
{
	JRB rooms;
	char buffer[4096];
	char *message;
	Client *client;
	Room *room;

	//Grabs the passed thread and the main tree
	client = (Client*) thread;
	rooms = main_room;

	//Calls the client helper function. This was to reduce the number of lines in a single function its is a void star. 
	clientHelper(rooms, client); 

	
	while(fgets(buffer, 4096, client->in) != NULL)
	{
		message = (char*) malloc(sizeof(char) * (strlen(buffer) + strlen(client->name) + 20));

		//Grabs the messages locks the mutex, appends to the list, call the condition signal and the proced to unlock the mutex.
		sprintf(message, "%s: %s", client->name, buffer); 
		pthread_mutex_lock(client->client_room->lock); 
		dll_append(client->client_room->messages, new_jval_s(message));
		pthread_cond_signal(client->client_room->condition); 
		pthread_mutex_unlock(client->client_room->lock);
	}

	//Indecates when a client has left/ 
	room = client->client_room;
	printf("Client %s has left\n", client->name);
	
	//Grabs and prints the messages and the appends them to the list. 
	message = (char*) malloc(sizeof(char) * (strlen(client->name) + 20));
	sprintf(message, "%s has left\n", client->name);
	dll_append(room->messages, new_jval_s(message));

	//Frees everything that needs to be free'd
	close(client->fd);
	fclose(client->in);
	fclose(client->out);
	free(client->name);
	dll_delete_node(client->curr_c_list);
	free(client);
	
	pthread_cond_signal(room->condition);
}

void clientHelper(JRB tree, Client *cc)
{
	char *message;
	JRB tmp_room;
	Dllist tmplist;
	Room *room;
	char buffer[4096];
	
	fprintf(cc->out, "Chat Rooms:\n");
	fflush(cc->out);

	//Traverses through the room trees and prints out all available rooms 
	jrb_traverse(tmp_room, tree)
	{
		room = ((Room*) tmp_room->val.v);
		
		fprintf(cc->out, "\n%s:", room->name);
		fflush(cc->out);

		dll_traverse(tmplist, room->clients)
		{
			fprintf(cc->out, " %s", ((Client*) tmplist->val.v)->name);
			fflush(cc->out);
		}

	}

	//Asks the user to enter their name 
	fprintf(cc->out, "\n\nEnter your chat name (no spaces):\n");
	fflush(cc->out);
	fgets(buffer, 4096, cc->in); 
	feof(cc->in);
	
	//Adds null character the end of the buffer. 
	buffer[strlen(buffer)-1] = '\0';
	cc->name = strdup(buffer);

	//Asks user what chat room they would like to enter
	fprintf(cc->out, "Enter chat room:\n");
	fflush(cc->out);
	fgets(buffer, 4096, cc->in); 
	feof(cc->in);

	//Adds null character the end of the buffer. 
	buffer[strlen(buffer)-1] = '\0';
	tmp_room = jrb_find_str(tree, buffer);

	//Error check to make sure the user uses a valid room and will keep promting the user to input a valid room. 
	while(tmp_room == NULL)
	{ 
		fprintf(cc->out, "No chat room %s\n", buffer);
		fflush(cc->out);
		fgets(buffer, 4096, cc->in); 
		feof(cc->in);

		buffer[strlen(buffer)-1] = '\0';

		tmp_room = jrb_find_str(tree, buffer);
	}
	
	//Grabs the current list of clients in the room
	cc->client_room = ((Room*) tmp_room->val.v);
	message = (char *) malloc(sizeof(char) * (strlen(cc->name) + 20));

	//Prints the message and when a user joins
	sprintf(message, "%s has joined\n", cc->name);

	//Appends the clients and the messages to their respected lists. 
	dll_append(cc->client_room->clients, new_jval_v(cc));
	dll_append(cc->client_room->messages, new_jval_s(message));
	
	//Grabs the last user to join the room
	cc->curr_c_list = dll_last(cc->client_room->clients);

	pthread_cond_signal(cc->client_room->condition);
}