#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <bits/pthreadtypes.h>
#include <sys/types.h>
#include <unistd.h>
#include "hashtable.h"
#define NTHREAD 32

struct thread_info {
  int id;
  void *(*start_routine) (void *);
  void *arg;
};

typedef struct lock_value {
  ll clock;
  HT *line_table;
} LOCK_V;

typedef struct line_value {
  void* lock_addr;
  void* line_addr;
  ll wait_cycles;
  ll run_cycles;
} LINE_V;


static LOCK_V* get_or_create_lock_entry(HT* ht, void* lock_addr) {
  LOCK_V *ret = get_entry(ht,lock_addr);
  if(ret == NULL) {
    ret = malloc(sizeof(LOCK_V));
    ret->line_table = init_table(CAPACITY);
    ret->clock = -1;
    set_entry(ht,lock_addr,ret);
  }
  return ret;
}

static LINE_V* get_or_create_line_entry(HT* ht, void* lock_addr, void* line_addr) {
  LINE_V *ret = get_entry(ht,line_addr);
  if(ret == NULL) {
    ret = malloc(sizeof(LINE_V));
    ret->lock_addr = lock_addr;
    ret->line_addr = line_addr;
    ret->wait_cycles = 0;
    ret->run_cycles = 0;
    set_entry(ht,line_addr,ret);
  }
  return ret;
}

//static int (*original_mutex_init)(pthread_mutex_t*) = NULL;
static int (*original_mutex_lock)(pthread_mutex_t*) = NULL;
//static int (*original_mutex_trylock)(pthread_mutex_t*) = NULL;
static int (*original_mutex_unlock)(pthread_mutex_t*) = NULL;
static int (*original_pthread_create)(pthread_t*, const pthread_attr_t*, void*(void*),void*) = NULL;

static __thread int counter = 1;
static __thread int id = 0;
static HT *table[NTHREAD];

static void con() __attribute__((constructor));
static void des() __attribute__((destructor));

void con() {
  int i;
  for(i = 0; i < NTHREAD; i++) {
    table[i] = init_table(CAPACITY);
  }
}

static void trim(char*);

static __thread FILE *out = NULL;

static void print_line_entry(void* value) {
  LINE_V *liv = value;
  fprintf(out, "%p,%p,%lld,%lld\n", liv->lock_addr, liv->line_addr, liv->wait_cycles, liv->run_cycles);
}

static void print_lock_entry(void* value) {
  LOCK_V *lov = value;
  map_table(lov->line_table, print_line_entry);
}

static void free_lock_entry(void* value) {
  LOCK_V *lov = value;
  free_table(lov->line_table);
}

void des() {
  char fname[100];
  char buf[100];
  snprintf(&fname, 100, "/proc/%d/maps", getpid());
  FILE *info = fopen(&fname, "r");
  fgets(&buf, 100, info);
  trim(&buf);
  int i;
  out = fopen("./result.csv", "w+");
  fprintf(out, "0x%s\n", &buf);
  for(i = 0; i < counter+1; i++) {
    map_table(table[i], print_lock_entry);
  }
  for(i = 0; i < NTHREAD; i++) {
    if(table[i]) {
      map_table(table[i], free_lock_entry);
      free_table(table[i]);
    }
  }
}

static inline uint64_t get_cycles()
{
  uint64_t t;
  __asm volatile ("rdtsc" : "=A"(t));
  return t;
}

void trim(char* buf) {
  for(;*buf != '-' && *buf != '\0'; buf++);
  for(;*buf != '\0'; buf++) *buf = '\0';
}

void * wrapper_routine(void *arg) {
  struct thread_info *tinfo = arg;
  id = tinfo->id;
  void *(*start_routine)(void*) = tinfo->start_routine;
  arg = tinfo->arg;
  free(tinfo);
  return start_routine(arg);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
  if(!original_pthread_create) original_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
  struct thread_info *tinfo = malloc(sizeof(struct thread_info));
  tinfo->start_routine = start_routine;
  tinfo->arg = arg;
  tinfo->id = counter;
  counter++;
  int i;
  i = original_pthread_create(thread, attr, wrapper_routine, (void*)tinfo);
  return i;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  if(!original_mutex_lock) original_mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  ll start = get_cycles();
  int ret = original_mutex_lock(mutex);
  ll end = get_cycles();
  if(end > start) {
    ll diff = end-start;
    void* ret_addr = __builtin_return_address(0);
    LOCK_V *lov = get_or_create_lock_entry(table[id], mutex);
    LINE_V *liv = get_or_create_line_entry(lov->line_table, mutex, ret_addr);
    liv->wait_cycles += diff;
    lov->clock = get_cycles();
  }
  return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  ll end_clock = get_cycles();
  if(!original_mutex_unlock) original_mutex_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  ll start_clock = -1;
  void* ret_addr = __builtin_return_address(0);
  LOCK_V *lov = get_or_create_lock_entry(table[id], mutex);
  LINE_V *liv = get_or_create_line_entry(lov->line_table, mutex, ret_addr);
  start_clock = lov->clock;
  if (start_clock > -1 && end_clock > start_clock) {
    ll diff = end_clock-start_clock;
    liv->run_cycles += diff;
    lov->clock = -1;
  }
  return original_mutex_unlock(mutex);
}

/*
 * mutex trylock
 */
