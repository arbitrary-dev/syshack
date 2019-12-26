#include <stdlib.h>

#include "llist.h"

Node *
l_prepend(Node *n, Node *to) {
  n->next = to;
  return n;
}

void
l_append(Node *n, Node *to) {
  while (to->next != NULL)
    to = to->next;
  to->next = n;
}
