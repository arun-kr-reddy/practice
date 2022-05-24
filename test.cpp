#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

typedef struct linked_list
{
    uint8_t value;
    linked_list *next;
} linked_list;

void printLL(linked_list *head)
{
    linked_list *current = head;

    while (current != NULL)
    {
        printf("%d\n", current->value);
        current = current->next;
    }
}

void deleteNode(linked_list *head, uint8_t idx)
{
    printf("idx %u\n", idx);
    head = NULL; // head->next;
    return;

    if (idx == 0)
    {
        printf("inside");
        head = NULL; // head->next;
        return;
    }

    uint8_t i = 0;
    linked_list *previous = NULL;
    linked_list *current = head;
    while (current != NULL)
    {
        previous = current;
        current = current->next;
        ++i;
        if (i == idx)
        {
            previous->next = current->next;
            return;
        }
    }
}

int main(void)
{
    linked_list head, middle, tail;

    head.value = 10;
    head.next = &middle;

    middle.value = 20;
    middle.next = &tail;

    tail.value = 30;
    tail.next = NULL;

    deleteNode(&head, 2);
    printf("head %u\n", head.value);
    printLL(&head);

    return 0;
}