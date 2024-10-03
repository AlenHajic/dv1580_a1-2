#include <stdio.h>
#include <stdint.h>
#include "memory_manager.h"  // Include your custom memory manager header.

typedef struct Node {
    uint16_t data; // Stores the data as an unsigned 16-bit integer
    struct Node* next; // A pointer to the next node in the List
} Node;

// Linked list initialization function
void list_init(Node** head, size_t size) {
    *head = NULL;  // Initialize the head to NULL to indicate an empty list.
    mem_init(size);  // Initialize the memory manager with the specified size.
}

// Insertion at the rear of the list
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));  // Allocate memory for the new node.
    if (new_node == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Insert a new node after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Error: Previous node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

// Insert a new node before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (*head == NULL || next_node == NULL) {
        printf("Error: List is empty or next_node is NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != next_node) {
            if (current->next == NULL) {
                printf("Error: next_node not found.\n");
                return;
            }
            current = current->next;
        }
        new_node->next = next_node;
        current->next = new_node;
    }
}

// Delete a node with the specified data
void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        printf("Error: List is empty.\n");
        return;
    }

    Node* current = *head;
    Node* previous = NULL;

    while (current != NULL && current->data != data) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Error: Node with data %u not found.\n", data);
        return;
    }

    if (previous == NULL) {
        *head = current->next;
    } else {
        previous->next = current->next;
    }

    mem_free(current);
}

// Search for a node with the specified data
Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;

    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// Display all elements in the linked list
void list_display(Node** head) {
    Node* current = *head;

    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        current = current->next;
        if (current != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Display elements in a specific range
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* current = (start_node == NULL) ? *head : start_node;

    printf("[");
    while (current != NULL) {
        printf("%u", current->data);

        if (current == end_node) {
            break;
        }

        current = current->next;
        if (current != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Count the number of nodes in the linked list
int list_count_nodes(Node** head) {
    Node* current = *head;
    int count = 0;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

// Cleanup the list and free all nodes
void list_cleanup(Node** head) {
    Node* current = *head;

    while (current != NULL) {
        Node* next = current->next;
        mem_free(current);
        current = next;
    }

    *head = NULL;
}
