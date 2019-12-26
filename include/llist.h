#ifndef LLIST_H
#define LLIST_H

typedef struct node {
  void *value;
  struct node *next;
} Node;

Node * l_prepend(Node *n, Node *to);
void l_append(Node *n, Node *to);

#endif
