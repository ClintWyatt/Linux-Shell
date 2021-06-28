#include "history.h"
#include <stdio.h>
void addEntry(struct commandHistory **head, struct commandHistory **last, int numRecords, int recordLimit, char *command)
{
	if(numRecords == 0)
	{
		memset(((*head)->command), '\0', sizeof((*head)->command));
		memcpy(((*head)->command), command, sizeof(command));
		last =head;
		(*last)->next = NULL;
	}
	else if(numRecords < recordLimit)
	{
		struct commandHistory *node = (struct commandHistory *)malloc(sizeof(struct commandHistory));
		memset(node->command, '\0', sizeof(node->command));
		memcpy(node->command, command, sizeof(node->command));
		node->next =*head;
		(*head)->previous = node;
		*head = node;
		(*head)->previous = NULL;
		if(numRecords == 1)
		{
			(*last)->previous = (*head);
		}
	}
	else
	{
		struct commandHistory *node = (struct commandHistory *)malloc(sizeof(struct commandHistory));
		memset(node->command, '\0', sizeof(node->command));
		memcpy(node->command, command, sizeof(node->command));
		node->next =*head;
		(*head)->previous =node;
		*head = node;
		(*head)->previous = NULL;
		struct commandHistory *lastTmp = (*last);
		*last = (*last)->previous;
		lastTmp->previous = NULL;
		(*last)->next = NULL;
		free(lastTmp);
	}
}

void deleteHistory(struct commandHistory *head, struct commandHistory *last)
{
	struct commandHistory *current;
	while(head != NULL)
	{
		current =head;
		head =head->next;
		current->next = NULL;
		current->previous = NULL;
		free(current);
	}
}

void print(struct commandHistory *head)
{
	struct commandHistory *current = NULL;
	current =head;
	while(current != NULL)
	{
		printf("%s", current->command);
		current = current->next;
	}
}

