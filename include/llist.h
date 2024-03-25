#ifndef LLIST_H
#define LLIST_H

typedef struct node {
  void *value;
  struct node *next;
} Node;

Node * l_new(void *head);

Node * l_prepend(Node *to, void *v);
Node * l_append(Node *to, void *v);

#endif
