#define CAPACITY 50
typedef long long int ll;
typedef struct item_t ITEM;
typedef struct hashtable_t HT;
typedef struct pthread_item_t PITEM;
typedef struct pthread_table_t PTT;

HT * init_table(int);
void free_table(HT*);
void add_entry_cycles(HT*,void*,ll,ll);
void set_entry_clock(HT*,void*,ll);
int get_entry_cycles(HT*,void*,ll*,ll*);
long long get_entry_clock(HT*,void*);
void display_table(FILE*,HT*);
