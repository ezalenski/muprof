#define CAPACITY 50
typedef long long int ll;
typedef struct item_t ITEM;
typedef struct hashtable_t HT;
typedef struct pthread_item_t PITEM;
typedef struct pthread_table_t PTT;

HT * init_table(int);
void free_table(HT*);
void set_entry(HT*,void*,void*);
void* get_entry(HT*,void*);
void map_table(HT*, void(*func)(void*));
