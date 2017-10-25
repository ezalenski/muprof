#include <pthread.h>

int main() {
  pthread_mutex_t p;
  pthread_mutex_init(&p, NULL);
  pthread_mutex_lock(&p);
  long long a;
  for(a = 0; a < 100; a++);
  pthread_mutex_unlock(&p);
  pthread_mutex_lock(&p);
  pthread_mutex_unlock(&p);
  return 0;
}
