#ifndef LLIST_H
#define LLIST_H

typedef struct node {
  void *value;
  struct node *next;
} Node;

Node * l_prepend(Node *to, Node *n);
Node * l_append(Node *to, Node *n);

#endif
