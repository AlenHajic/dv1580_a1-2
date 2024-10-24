#include "memory_manager.h"
#include "linked_list.h"

//We initiallize the linked list
void list_init(Node** head, size_t size) {
    *head = NULL; //defererar head för att få dess värde, alltså adressen till den försat noden
    size_t total_pool_size = sizeof(Node) * size; //Vi räknar ut hur mycket memory som behövs för varje 
    mem_init(total_pool_size);
}

void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node)); //Allocating memory for a new node

    if (new_node == NULL) { //Om noden är NUll så gick det inte att skapa den
        printf("Error: Memory allocation failed.\n");
        return;
    }

    new_node->data = data; //Noden får den datan den skulle ha
    new_node->next = NULL; //Next ptr:n pekar inte på något eftersom den är sist i listan

    if (*head == NULL) { //Om head är NULL så längs den in på den nya noden
        *head = new_node;
    } else {
        Node* current = *head; //create a current node that is the start of our loop
        while (current->next != NULL) //Check does our current node have a next, if yes then contiune if not then we are at the end and insert our new node
        {
            current = current->next; 
        }
        current->next = new_node; //we make 
    }

    // Print confirmation for debugging (optional)
    printf("Inserted node with data: %u\n", data);
}


// Insert a new node after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) { //Den noden vi vill lägga in en ny node efter kan inte vara NULL eftersom den då inte finns
        printf("Error: Previous node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node)); //Allokerar nytt minne för den nya noden
    if (new_node == NULL) { //Kollar om det gick att allokera den
        printf("Error: Memory allocation failed.\n");
        return;
    }

    new_node->data = data; //Lägger in önskad data
    new_node->next = prev_node->next; //Eftersom new_node ska in innan prev_node så måste prev_nodes next bli new_nodes next
    prev_node->next = new_node; //new_node hamnar efter prev_node
}

// Insert a new node before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (*head == NULL || next_node == NULL) { //om head är null finns ingen node att lägga in innan och om next_node är null för isf kan vi inte fortsätta
        printf("Error: List is empty or next_node is NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node)); //Allokerar en ny node
    if (new_node == NULL) { //Kollar om det gick att allokera den nya noden
        printf("Error: Memory allocation failed.\n");
        return;
    }

    new_node->data = data; //Ändrar till önskad data

    //Kollar om next_node är head isf ska head bli new_node-> next och head ska bli vår new_node
    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else { //Annars letar vi efter våra next_node tills vi hittar den.
        Node* current = *head;
        while (current->next != next_node) {
            if (current->next == NULL) {
                printf("Error: next_node not found.\n");
                return;
            }
            current = current->next; 
        }
        new_node->next = next_node; //Lägger next_node som new_nodes next
        current->next = new_node; //new_node där next_node var innan
    }
}

// Delete a node with the specified data
void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) { //Listan är tom
        printf("Error: List is empty.\n");
        return;
    }

    Node* current = *head; //current blir head
    Node* previous = NULL; //använder för att hjälpa re-linka när vi hittat våran node vi ska ta bort

    while (current != NULL && current->data != data) { //första vilkoret ser till att vi fortsätter så länge listan ahr nodes kvar, andra vilkoret kör så länge vi inte hittat en node med den datan vi vill ta bort
        previous = current; //previos lyttas ett steg år höger
        current = current->next; //current flyttas ett steg åt höger
    }

    if (current == NULL) { //Om current är NULL finns inte det vi letar efter
        printf("Error: Node with data %u not found.\n", data);
        return;
    }

    if (previous == NULL) { //We check if current and previous were able to move, if previous was not able to move head is the one we want to delete.
        *head = current->next; //Move head so that we can delete it and get a new head
    } else {
        previous->next = current->next; //current är den vi vill ta bort så ett glapp komemr skapas när vi tar bort denna och därför måste vi koppla tillbaka listan genom att läga previous next som den efter current
    }

    mem_free(current); //freeing memeory from the node we wanted to delete
}

// Search for a node with the specified data
Node* list_search(Node** head, uint16_t data) {
    Node* current = *head; //Current för att leta efter en node

    while (current != NULL) { //Så länge det finsn nodes fortsätter den
        if (current->data == data) { //Om currents data är den datan på den noden vi letar efter har vi hittat rätt.
            return current;
        }
        current = current->next; //Annars fortsätter vi leta igenom våran linked-list
    }

    return NULL; 
}

// Display all elements in the linked list
void list_display(Node** head) {
    Node* current = *head; //En current för att traversera igenom våran linked list

    // Start the list display with the opening bracket
    printf("[");

    while (current != NULL) {
        // Print the current node's data
        printf("%u", current->data);

        // If this is not the last node, print a comma and space
        if (current->next != NULL) { //Kollar om det är våran sista node vi kommit till eller inte
            printf(", ");
        }

        // Move to the next node
        current = current->next;
    }

    // Close the list display with the closing bracket
    printf("]");
}

// Display elements in a specific range
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* current = (start_node == NULL) ? *head : start_node; //sätter startpunkten men beroden på om start_node är NULL sätter vi den som *head annars är det start_node

    // Start the list display with the opening bracket
    printf("[");

    while (current != NULL) {
        // Print the current node's data
        printf("%u", current->data);

        // Stop if we've reached the end node
        if (current == end_node) {
            break;
        }

        // Move to the next node
        current = current->next;

        // If this is not the last node, print a comma and space
        if (current != NULL) {
            printf(", ");
        }
    }

    // Close the list display with the closing bracket
    printf("]");
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
