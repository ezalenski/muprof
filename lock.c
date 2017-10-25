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

//static int (*original_mutex_init)(pthread_mutex_t*) = NULL;
static int (*original_mutex_lock)(pthread_mutex_t*) = NULL;
//static int (*original_mutex_trylock)(pthread_mutex_t*) = NULL;
static int (*original_mutex_unlock)(pthread_mutex_t*) = NULL;
static int (*original_pthread_create)(pthread_t*, pthread_attr_t*, void*(void*),void*) = NULL;

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

void des() {
  printf("%d\n", getpid());
  int i;
  FILE* out = fopen("./result.csv","w+");
  fprintf(out, "addr,wait,run\n");
  for(i = 0; i < counter+1; i++) {
    display_table(out, table[i]);
  }
  for(i = 0; i < NTHREAD; i++) {
    if(table[i])
      free_table(table[i]);
  }
}

static inline uint64_t get_cycles()
{
  uint64_t t;
  __asm volatile ("rdtsc" : "=A"(t));
  return t;
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
    add_entry_cycles(table[id],(void*)mutex,diff,0);
    add_entry_cycles(table[id],__builtin_return_address(0),diff,0);
    set_entry_clock(table[id],(void*)mutex,get_cycles());
  }
  return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  ll end_clock = get_cycles();
  if(!original_mutex_unlock) original_mutex_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  ll start_clock = -1;
  start_clock = get_entry_clock(table[id],(void*)mutex);
  if (start_clock > -1 && end_clock > start_clock) {
    ll diff = end_clock-start_clock;
    add_entry_cycles(table[id],(void*)mutex,0,diff);
    add_entry_cycles(table[id],__builtin_return_address(0),0,diff);
    set_entry_clock(table[id],(void*)mutex,-1);
  }
  return original_mutex_unlock(mutex);
}

/*
 * mutex trylock
 */
