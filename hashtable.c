#include <stdlib.h>
#include <stdio.h>
#include "hashtable.h"

struct item_t {
  void* key;
  ll wait_cycles;
  ll run_cycles;
  ll clock;
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
        free(prev);
      }
    }
  }
  free(htable->table);
  free(htable);
}

static ITEM* get_or_create_entry(HT* htable, void* key) {
  size_t index = ((size_t)key)%(htable->capacity);
  ITEM * head = htable->table[index];
  ITEM * prev = NULL;
  for(;head && head->key != key; prev = head, head = head->next);
  if(head == NULL) {
    head = malloc(sizeof(ITEM));
    head->key = key;
    head->wait_cycles = 0;
    head->run_cycles = 0;
    head->clock = -1;
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

static ITEM* get_entry(HT* htable, void* key) {
  size_t index = ((size_t)key)%(htable->capacity);
  ITEM * head = htable->table[index];
  for(;head && head->key != key; head = head->next);
  return head;
}

void set_entry_clock(HT* htable, void* key, ll clock) {
  ITEM *head = get_or_create_entry(htable, key);
  head->clock = clock;
}

void add_entry_cycles(HT* htable, void* key, ll wait_cycles, ll run_cycles) {
  ITEM *head = get_or_create_entry(htable, key);
  head->wait_cycles += wait_cycles;
  head->run_cycles += run_cycles;
}

int get_entry_cycles(HT * htable, void* key, ll* wait_cycles, ll* run_cycles) {
  ITEM *head = get_entry(htable, key);
  *wait_cycles = head ? head->wait_cycles : 0;
  *run_cycles = head ? head->run_cycles : 0;
  return head ? 1 : 0;
}

ll get_entry_clock(HT* htable, void* key) {
  ITEM *head = get_entry(htable, key);
  return head ? head->clock : -1;
}

void display_table(FILE* f, HT* htable) {
  for(int i = 0; i < htable->capacity; i++) {
    for(ITEM* head = htable->table[i];head != NULL; head = head->next) {
      fprintf(f, "%p,%lld,%lld\n", head->key, head->wait_cycles, head->run_cycles);
    }
  }
}
