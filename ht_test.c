#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "hashtable.h"

struct data {
  ll wait;
  ll run;
  ll clock;
};

static void print_data(void* value) {
  struct data *d = value;
  printf("%lld, %lld, %lld\n", d->wait, d->run, d->clock);
}

void main() {
  ll i;
  HT * htable = init_table(50);
  for(i = 0; i < 10; i++) {
    struct data *d, *g;
    d = malloc(sizeof(struct data));
    d->clock = i*100;
    d->wait = 0;
    d->run = 0;
    set_entry(htable,(void*)i,d);
    g = get_entry(htable, (void*)i);
    g->run += i;
    printf("entry %lld: %lld, %lld\n", i, g->wait, g->run);
    d->wait = i;
    d->run += 1;
    g = get_entry(htable, (void*)i);
    printf("entry %lld: %lld, %lld\n", i, g->wait, g->run);
    printf("clock %lld\n", g->clock);
  }
  map_table(htable, print_data);
  free_table(htable);
}
