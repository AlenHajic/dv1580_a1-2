#include "linked_list.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stddef.h>

#include "common_defs.h"
#include "gitdata.h"

// Function to capture stdout output.
void capture_stdout(char *buffer, size_t size, void (*func)(Node **, Node *, Node *), Node **head, Node *start_node, Node *end_node)
{
    // Save the original stdout
    FILE *original_stdout = stdout;

    // Open a temporary file to capture stdout
    FILE *fp = tmpfile(); // tmpfile() creates a temporary file
    if (fp == NULL)
    {
        printf("Failed to open temporary file for capturing stdout.\n");
        return;
    }

    // Redirect stdout to the temporary file
    stdout = fp;

    // Call the function whose output we want to capture
    func(head, start_node, end_node);

    // Flush the output to the temporary file
    fflush(fp);

    // Reset the file position to the beginning
    rewind(fp);

    // Read the content of the temporary file into the buffer
    fread(buffer, 1, size - 1, fp); // Leave space for null terminator
    buffer[size - 1] = '\0';        // Ensure the buffer is null-terminated

    // Close the temporary file
    fclose(fp);

    // Restore the original stdout
    stdout = original_stdout;
}

// ********* Test basic linked list operations *********

void test_list_init()
{
    printf_yellow("  Testing list_init ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node));
    my_assert(head == NULL);
    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_insert()
{
    printf_yellow("  Testing list_insert ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 2);
    list_insert(&head, 10);
    list_insert(&head, 20);
    my_assert(head->data == 10);
    my_assert(head->next->data == 20);
    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_insert_after()
{
    printf_yellow("  Testing list_insert_after ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 3);
    list_insert(&head, 10);
    Node *node = head;
    list_insert_after(node, 20);
    my_assert(node->next->data == 20);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_insert_before()
{
    printf_yellow("  Testing list_insert_before ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 3);
    list_insert(&head, 10);
    list_insert(&head, 30);
    Node *node = head->next; // Node with data 30
    list_insert_before(&head, node, 20);
    my_assert(head->next->data == 20);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_delete()
{
    printf_yellow("  Testing list_delete ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 2);
    list_insert(&head, 10);
    list_insert(&head, 20);
    list_delete(&head, 10);
    my_assert(head->data == 20);
    list_delete(&head, 20);
    my_assert(head == NULL);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_search()
{
    printf_yellow("  Testing list_search ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 2);
    list_insert(&head, 10);
    list_insert(&head, 20);
    Node *found = list_search(&head, 10);
    my_assert(found->data == 10);

    Node *not_found = list_search(&head, 30);
    my_assert(not_found == NULL);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_display()
{
    printf_yellow("  Testing list_display ... \n");
    Node *head = NULL;

    int Nnodes = 5 + rand() % 5;
#ifdef DEBUG
    printf_yellow("   Testing %d nodes.\n", Nnodes);
#endif

    list_init(&head, sizeof(Node) * Nnodes);

    int randomLow = rand() % Nnodes;

    //    randomLow=0;

    int randomHigh = randomLow + rand() % (Nnodes - randomLow);
    while (randomHigh == 0)
    {
        randomHigh = randomLow + rand() % (Nnodes - randomLow);
    }

#ifdef DEBUG
    int Delta = randomHigh - randomLow;
    printf("Random [%d,%d] delta= %d \n", randomLow, randomHigh, Delta);
#endif

    char *stringFull = malloc(1024);
    char *string2Last = malloc(1024);
    char *string1third = malloc(1024);
    char *stringRandom = malloc(1024);

    sprintf(stringFull, "[");
    sprintf(string2Last, "[");

    Node *Low = NULL;
    Node *High = NULL;
    char LowValue[10];
    char HighValue[10];

    int values[Nnodes];
    for (int i = 0; i < Nnodes; i++)
    {
        values[i] = 0;
    }
    for (int k = 0; k < Nnodes; k++)
    {
        values[k] = 10 + rand() % 90;
        list_insert(&head, values[k]);
        if (k == randomLow && !Low)
        {
            Low = list_search(&head, values[k]);
            sprintf(LowValue, "%d", values[k]);
        }
        if (k == randomHigh && !High)
        {
            High = list_search(&head, values[k]);
            sprintf(HighValue, "%d", values[k]);
        }
        sprintf(stringFull + strlen(stringFull), "%d", values[k]);
        if (k < (Nnodes - 1))
        {
            sprintf(stringFull + strlen(stringFull), ", ");
        }
        else
        {
            sprintf(stringFull + strlen(stringFull), "]");
        }
    }

#ifdef DEBUG
    printf("LowValue=%s, HighValue=%s\n", LowValue, HighValue);
    printf("RefFull:'%s'\n", stringFull);
#endif

    // [N0, N1, ...., NL]
    sprintf(string2Last + 1, "%s", strchr(stringFull, ',') + 2);

    //    printf("ref2Last: '%s\n", string2Last);

    // [N0, N1, ...., NL]
    char *Third = stringFull;
    for (int i = 0; i < 3; i++)
    {
        Third = strchr(Third, ',');
        Third += 1;
    }
    int LenToThird = ((Third - stringFull));

    strncpy(string1third, stringFull, LenToThird - 1);
    sprintf(string1third + strlen(string1third), "]");

    // printf("ref1third: '%s\n", string1third);

    char *start = 0;
    char *first = 0;
    char *last = 0;

    // Find first random node.
    start = strstr(stringFull, LowValue);
    first = strstr(stringFull, HighValue);
    if (strlen(first) > 3)
    {
        // We have atleast one number after us.
#ifdef DEBUG
        printf("Last isnt last.\n");
#endif
        last = strstr(first, " ");
    }
    else
    {
        // We are the last number.
#ifdef DEBUG
        printf("Last was last.\n");
#endif
        last = strstr(first, "]");
    }

    ptrdiff_t LenToFirst = ((char *)start - (char *)stringFull);
    ptrdiff_t LenToLast = ((char *)last - (char *)stringFull);

#ifdef DEBUG
    printf("random starts at %p (offset=%ld) and ends at %p (offset=%ld).\n", start, LenToFirst, last, LenToLast);
    printf("random: '%s' \n", start);

#endif

    char *blob = malloc(1024);
    strncpy(blob, start, LenToLast - LenToFirst);

    sprintf(stringRandom, "[%s", blob);
    if (!strchr(stringRandom, ']'))
    {
        sprintf(stringRandom + strlen(stringRandom), "]");
    }
    if (strstr(stringRandom, ",]"))
    {
        // Solution change ",]" to "]\0";
        char *ptr = strstr(stringRandom, ",]");
        sprintf(ptr, "]");
        memset(ptr + 1, 0, 1);
#ifdef DEBUG
        printf("We have a problem Huston.\n");
        printf("Fixed ,] issue.\n");
#endif
    }

    if (strstr(stringRandom, ", ]"))
    {
#ifdef DEBUG
        printf("We have a problem Huston2.\n");
#endif
    }

    //    printf("RefRandom: '%s' \n\n", stringRandom);

    char buffer[1024] = {0}; // Buffer to capture the output

    // Test case 1: Displaying full list
    capture_stdout(buffer, sizeof(buffer), (void (*)(Node **, Node *, Node *))list_display_range, &head, NULL, NULL);
    my_assert(strcmp(buffer, stringFull) == 0);
    printf("\tFull list: %s\n", buffer);

    // Test case 2: Displaying list from second node to end
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    capture_stdout(buffer, sizeof(buffer), (void (*)(Node **, Node *, Node *))list_display_range, &head, head->next, NULL);
    my_assert(strcmp(buffer, string2Last) == 0);
    printf("\tFrom second node to end: %s\n", buffer);

    // Test case 3: Displaying list from first node to third node
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    capture_stdout(buffer, sizeof(buffer), (void (*)(Node **, Node *, Node *))list_display_range, &head, head, head->next->next);
    my_assert(strcmp(buffer, string1third) == 0);
    printf("\tFrom first node to third node: %s\n", buffer);

    // Test case 4: Displaying random nodes
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    capture_stdout(buffer, sizeof(buffer), (void (*)(Node **, Node *, Node *))list_display_range, &head, Low, High);
    my_assert(strcmp(buffer, stringRandom) == 0);
    printf("\tK random node(s): %s\n", buffer);

    list_cleanup(&head);
    printf_green("  ... [PASS].\n");
}

void test_list_count_nodes()
{
    printf_yellow("  Testing list_count_nodes ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 3);
    list_insert(&head, 10);
    list_insert(&head, 20);
    list_insert(&head, 30);

    int count = list_count_nodes(&head);
    my_assert(count == 3);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_cleanup()
{
    printf_yellow("  Testing list_cleanup ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 3);
    list_insert(&head, 10);
    list_insert(&head, 20);
    list_insert(&head, 30);

    list_cleanup(&head);
    my_assert(head == NULL);
    printf_green("[PASS].\n");
}

// ********* Stress and edge cases *********

void test_list_insert_loop(int count)
{
    printf_yellow("  Testing list_insert loop ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * count);
    for (int i = 0; i < count; i++)
    {
        list_insert(&head, i);
    }

    Node *current = head;
    for (int i = 0; i < count; i++)
    {
        my_assert(current->data == i);
        current = current->next;
    }

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_insert_after_loop(int count)
{
    printf_yellow("  Testing list_insert_after loop ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * (count + 1));
    list_insert(&head, 12345);

    Node *node = list_search(&head, 12345);
    for (int i = 0; i < count; i++)
    {
        list_insert_after(node, i);
    }

    Node *current = head;
    my_assert(current->data == 12345);
    current = current->next;

    for (int i = count - 1; i >= 0; i--)
    {
        my_assert(current->data == i);
        current = current->next;
    }

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_delete_loop(int count)
{
    printf_yellow("  Testing list_delete loop ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * count);
    for (int i = 0; i < count; i++)
    {
        list_insert(&head, i);
    }

    for (int i = 0; i < count; i++)
    {
        list_delete(&head, i);
    }

    my_assert(head == NULL);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_search_loop(int count)
{
    printf_yellow("  Testing list_search loop ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * count);
    for (int i = 0; i < count; i++)
    {
        list_insert(&head, i);
    }

    for (int i = 0; i < count; i++)
    {
        Node *found = list_search(&head, i);
        my_assert(found->data == i);
    }

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

void test_list_edge_cases()
{
    printf_yellow("  Testing list edge cases ---> ");
    Node *head = NULL;
    list_init(&head, sizeof(Node) * 3);

    // Insert at head
    list_insert(&head, 10);
    my_assert(head->data == 10);

    // Insert after
    Node *node = list_search(&head, 10);
    list_insert_after(node, 20);
    my_assert(node->next->data == 20);

    // Insert before
    list_insert_before(&head, node, 15);

    my_assert(head->data == 15);
    my_assert(head->next->data == 10);
    my_assert(head->next->next->data == 20);

    // Delete
    list_delete(&head, 15);
    my_assert(node->next->data == 20);

    // Search
    Node *found = list_search(&head, 20);
    my_assert(found->data == 20);

    list_cleanup(&head);
    printf_green("[PASS].\n");
}

// Main function to run all tests
int main(int argc, char *argv[])
{

    srand(time(NULL));
#ifdef VERSION
    printf("Build Version; %s \n", VERSION);
#endif
    printf("Git Version; %s/%s \n", git_date, git_sha);
    if (argc < 2)
    {
        printf("Usage: %s <test function>\n", argv[0]);
        printf("Available test functions:\n");
        printf("Basic Operations:\n");
        printf(" 1. test_list_init - Initialize the linked list\n");
        printf(" 2. test_list_insert - Test basic list insert operations");
        printf(" 3. test_list_insert_after - Test list insert after a given node\n");
        printf(" 4. test_list_insert_before - Test list insert before a given node\n");
        printf(" 5. test_list_delete - Test delete operation\n");
        printf(" 6. test_list_search - Test search for a particular node\n");
        printf(" 7. test_list_display - Test the display functionality. Requires subjective validation\n");
        printf(" 8. test_list_count_nodes - Test nodes count function\n");
        printf(" 9. test_list_cleanup - Test clean up\n");

        printf("\nStress and Edge Cases:\n");
        printf(" 10. test_list_insert_loop - Test multiple insertions\n");
        printf(" 11. test_list_insert_after_loop - Test multiple insertions after a given node\n");
        printf(" 12. test_list_delete_loop - Test multiple detelions\n");
        printf(" 13. test_list_search_loop - Test multiple search\n");
        printf(" 14. test_list_edge_cases - Test edge cases\n");
        printf(" 0. Run all tests\n");
        return 1;
    }

    switch (atoi(argv[1]))
    {
    case -1:
        printf("No tests will be executed.\n");
        break;
    case 0:
        printf("Testing Basic Operations:\n");
        test_list_init();
        test_list_insert();
        test_list_insert_after();
        test_list_insert_before();
        test_list_delete();
        test_list_search();
        test_list_display();
        test_list_count_nodes();
        test_list_cleanup();

        printf("\nTesting Stress and Edge Cases:\n");
        test_list_insert_loop(1000);
        test_list_insert_after_loop(1000);
        test_list_delete_loop(1000);
        test_list_search_loop(1000);
        test_list_edge_cases();
        break;
    case 1:
        test_list_init();
        break;
    case 2:

        test_list_insert();
        break;
    case 3:
        test_list_insert_after();
        break;
    case 4:
        test_list_insert_before();
        break;
    case 5:
        test_list_delete();
        break;
    case 6:
        test_list_search();
        break;
    case 7:
        test_list_display();
        break;
    case 8:
        test_list_count_nodes();
        break;
    case 9:
        test_list_cleanup();
        break;
    case 10:
        test_list_insert_loop(1000);
        break;
    case 11:
        test_list_insert_after_loop(1000);
        break;
    case 12:
        test_list_delete_loop(1000);
        break;
    case 13:
        test_list_search_loop(1000);
        break;
    case 14:
        test_list_edge_cases();
        break;

    default:
        printf("Invalid test function\n");
        break;
    }

    return 0;
}