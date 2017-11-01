#include <stdlib.h>
#include <stdio.h>
#include "hashtable.h"

struct item_t {
  void* key;
  void* value;
  struct item_t *next;
};

struct hashtable_t {
  ITEM ** table;
  int capacity;
  int size;
};

HT * init_table(int capacity) {
  HT * ret = malloc(sizeof(HT));
  ret->capacity = capacity;
  ret->table = malloc(sizeof(ITEM*)*capacity);
  for(int i = 0; i < capacity; i++) ret->table[i] = NULL;
  return ret;
}

void free_table(HT* htable) {
  int i;
  for(i = 0; i < htable->capacity; i++) {
    if(htable->table[i]) {
      ITEM* curr = NULL;
      ITEM* prev = NULL;
      for(curr = htable->table[i]; curr;) {
        prev = curr;
        curr = curr->next;
        if(prev->value != NULL) free(prev->value);
        free(prev);
      }
    }
  }
  free(htable->table);
  free(htable);
}

static ITEM* get_or_create_entry(HT* htable, void* key) {
  size_t index = ((size_t)key)%(htable->capacity);
  ITEM * head;
  ITEM * prev = NULL;
  for(head = htable->table[index]; head && head->key != key; prev = head, head = head->next);
  if(head == NULL) {
    head = malloc(sizeof(ITEM));
    head->key = key;
    head->value = NULL;
    head->next = NULL;
    htable->size++;
    if(prev == NULL) {
      htable->table[index] = head;
    } else {
      prev->next = head;
    }
  }
  return head;
}

static ITEM* get_entry_item(HT* htable, void* key) {
  size_t index = ((size_t)key)%(htable->capacity);
  ITEM * head;
  for(head = htable->table[index]; head && head->key != key; head = head->next);
  return head;
}

void set_entry(HT* htable, void* key, void* value) {
  ITEM *head = get_or_create_entry(htable, key);
  if(head->value != NULL) free(head->value);
  head->value = value;
}

void* get_entry(HT* htable, void* key) {
  ITEM *head = get_entry_item(htable, key);
  return head ? head->value : NULL;
}


void map_table(HT* htable, void(*func)(void*)) {
  for(int i = 0; i < htable->capacity; i++) {
    for(ITEM* head = htable->table[i];head != NULL; head = head->next) {
      func(head->value);
    }
  }
}
