#include <stdlib.h>

#include "llist.h"

Node *
l_prepend(Node *to, Node *n) {
  n->next = to;
  return n;
}

Node *
l_append(Node *to, Node *n) {
  if (!to)
    return n;
  while (to->next)
    to = to->next;
  to->next = n;
  return to;
}
