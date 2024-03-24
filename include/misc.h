#ifndef MISC_H
#define MISC_H

typedef enum {
  CHARACTER,
  ITEM,
} Type;

typedef struct {
  Type type;
} Object;

#endif
