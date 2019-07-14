#include <stdlib.h>
#include <stdint.h>
#include <string.h>

uint8_t *space = 0;

int create (size_t n) {	//битовый массив не более 1\8 памяти
   space = (uint8_t*)malloc((n + 7) / 8);
   if (space == 0)
	   return 1;
   memset (space, 0x00, (n +7) / 8);
   return 0;
}
int get (size_t n) {
	size_t bit = n % 8;
	size_t byte = n / 8;
	uint8_t mask = 1<<bit;
	if((space[byte] & mask) == 0)
		return 0;
	return 1;
}	
void set (size_t n, int val) {
	size_t bit = n % 8;
	size_t byte = n / 8;
	uint8_t mask = 1<<bit;
	if (val != 0) 
		space[byte] |= mask;
	else space[byte] &= ~mask;
}
