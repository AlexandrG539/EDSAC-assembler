#include <stdint.h>
int nameTableAdd (const char *name, uint16_t value); 
int nameTableSearch (const char *name, uint16_t *valueDst);
int printTable();
uint16_t getAdress (const char *name);

