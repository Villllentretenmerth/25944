#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1000

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

int main(void){
    Node *head = NULL;
    Node *tail = NULL;
    char buffer[BUFSIZE];

    int len = 0;
    Node *node;
    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            
            break;
        }
        if (buffer[0] == '.')
            break;

        len = strlen(buffer);

        node = (Node *)malloc(sizeof(Node));
        if (!node) {
            perror("malloc");
        }
        node->str = (char *)malloc(len + 1);   
        if (!node->str) {
            perror("malloc");
            free(node);
        }
        strcpy(node->str, buffer);     
        node->next = NULL;

        if (tail) {
            tail->next = node;
            tail = node;
        } else {
            head = tail = node;
        }
    }

    int first = 1;
    for (Node *cur = head; cur != NULL; cur = cur->next) {
        len = strlen(cur->str);
        if (len && cur->str[len-1] == '\n') len--;
        if (first) {
            fwrite(cur->str, 1, len, stdout);
            first = 0;
        } 
        else {
            fputs("\n", stdout);
            fwrite(cur->str, 1, len, stdout);
        }
    }
    if (!first) {
         putchar('\n');
    }
    
    Node *next;
    for (Node *cur = head; cur != NULL; ) {
        next = cur->next;
        free(cur->str);
        free(cur);
        cur = next;
    }

    return 0;
}
