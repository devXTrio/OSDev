/* The IPC Queue manager header file.

   
*/

#ifndef IPC_QUEUE_MANAGER_H
#define IPC_QUEUE_MANAGER_H

#include "ipc_definitions.h"

void deinit();
void init_queue(enum MESSAGE_QUEUES message_queue_enum);
struct queue_message_t *get_message_ipc(enum MESSAGE_QUEUES message_queue_enum);
struct message *find_nonempty_ipc(struct queue_message_t *queue);
int enqueue_ipc(enum MESSAGE_QUEUES source, enum MESSAGE_QUEUES destination, char *string);
void clear_ipc(struct message *process);
struct message *find_message_ipc(enum MESSAGE_QUEUES message_queue_enum, int pid);
struct message dequeue_ipc(enum MESSAGE_QUEUES message_queue_enum);
struct message delete1(enum MESSAGE_QUEUES message_queue_enum, struct message *temp);
int has_message(enum MESSAGE_QUEUES check);

#endif
