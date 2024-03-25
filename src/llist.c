#include <assert.h>
#include <stdlib.h>

#include "llist.h"

Node *
l_new(void *head)
{
	assert(head);
	Node *n  = malloc(sizeof(Node));
	n->value = head;
	n->next  = NULL;
	return n;
}

Node *
l_prepend(Node *to, void *v)
{
	Node *n = l_new(v);
	n->next = to;
	return n;
}

Node *
l_append(Node *to, void *v)
{
	if (!to) {
		return l_new(v);
	}

	while (to->next) {
		to = to->next;
	}
	to->next = l_new(v);

	return to;
}
