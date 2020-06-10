#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <stdlib.h>

struct vector;
struct record;

typedef void *continuation;

typedef struct value {
  int type;
  union {
    int integer_val;
    char *string_val;
    int symbol_val;
    char *list_val;
    continuation k_val;
    struct vector *vector_val;
    struct record *record_val;
    struct record *env_val;
  };
} value;

typedef struct vector {
  size_t capacity;
  size_t size;
  value *items;
} vector;

typedef struct entry {
  value name;
  value value;
} entry;

typedef struct table {
  int size;
  entry *entries;
} args;

typedef struct record {
  struct env *parent;
  int capacity;
  int size;
  entry *entries;
} record, env;

typedef void (*continuation_fn)(value *, continuation, env *);

#endif
