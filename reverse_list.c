#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

static Node *push_front(Node *head, int value) {
    Node *node = malloc(sizeof(*node));
    if (node == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    node->value = value;
    node->next = head;
    return node;
}

static Node *reverse_list(Node *head) {
    Node *prev = NULL;
    Node *current = head;

    while (current != NULL) {
        Node *next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    return prev;
}

static void print_list(const Node *head) {
    const Node *current = head;

    while (current != NULL) {
        printf("%d", current->value);
        if (current->next != NULL) {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
}

static void free_list(Node *head) {
    while (head != NULL) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

int main(void) {
    Node *head = NULL;

    for (int i = 5; i >= 1; --i) {
        head = push_front(head, i);
    }

    printf("Original list: ");
    print_list(head);

    head = reverse_list(head);

    printf("Reversed list: ");
    print_list(head);

    free_list(head);
    return 0;
}
