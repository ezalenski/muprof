#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "hashtable.h"

void main() {
  ll i;
  HT * htable = init_table(50);
  for(i = 0; i < 10; i++) {
    ll w, r;
    ll clock = i*100;
    add_entry_cycles(htable,(void*)i,0,i);
    if(!get_entry_cycles(htable, (void*)i, &w, &r)) {
      printf("failed\n");
    }
    printf("entry %lld: %lld, %lld\n", i, w, r);
    add_entry_cycles(htable,(void*)i,i,1);
    if(!get_entry_cycles(htable, (void*)i, &w, &r)) {
      printf("failed\n");
    }
    printf("entry %lld: %lld, %lld\n", i, w, r);
    set_entry_clock(htable, (void*)i, clock);
    printf("clock %lld\n", get_entry_clock(htable, (void*)i));
  }
  free_table(htable);
}
