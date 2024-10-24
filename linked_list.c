#include "memory_manager.h"
#include "linked_list.h"
#include <pthread.h>

// Mutex for thread-safe linked list operations
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the linked list
void list_init(Node** head, size_t size) {
    *head = NULL;
    size_t total_pool_size = sizeof(Node) * size;
    mem_init(total_pool_size);
}

// Insert a new node
void list_insert(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);  // Lock the mutex

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed.\n");
        pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
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

    pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
}

// Insert a new node after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    pthread_mutex_lock(&list_mutex);  // Lock the mutex

    if (prev_node == NULL) {
        printf("Error: Previous node cannot be NULL.\n");
        pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed.\n");
        pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
        return;
    }

    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;

    pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
}

// Delete a node with the specified data
void list_delete(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);  // Lock the mutex

    if (*head == NULL) {
        printf("Error: List is empty.\n");
        pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
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
        pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
        return;
    }

    if (previous == NULL) {
        *head = current->next;
    } else {
        previous->next = current->next;
    }

    mem_free(current);
    pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
}

// Search for a node with the specified data
Node* list_search(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);  // Lock the mutex

    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
    return NULL;
}

// Display the list
void list_display(Node** head) {
    pthread_mutex_lock(&list_mutex);  // Lock the mutex

    Node* current = *head;
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]");

    pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
}

// Clean up the linked list
void list_cleanup(Node** head) {
    pthread_mutex_lock(&list_mutex);  // Lock the mutex

    Node* current = *head;
    while (current != NULL) {
        Node* next = current->next;
        mem_free(current);
        current = next;
    }
    *head = NULL;

    pthread_mutex_unlock(&list_mutex);  // Unlock the mutex
}
