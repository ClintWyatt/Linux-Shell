#include <string.h>
#include <stdlib.h>
struct commandHistory
{
        char command[256];
        struct commandHistory *next;
        struct commandHistory *previous;
};

void addEntry(struct commandHistory **head, struct commandHistory *last, int numRecords, int recordLimit, char *command);

void deleteHistory(struct commandHistory *head, struct commandHistory *last);


void print(struct commandHistory *head);
