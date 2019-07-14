#include "nameTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct entry {
	struct entry *next;
	const char *name;
	uint16_t value;
};

struct entry *nameTable = 0;

int nameTableAdd (const char *name, uint16_t value) {
	if (0 == nameTableSearch (name, 0))
		return 2; // метка уже присутствует в таблице символов
	struct entry *newEntry = (struct entry *)calloc (sizeof (struct entry), 1);
	if (newEntry == 0) 
		return 1; //не удалось выделить память
	newEntry->name = strdup (name);
	if (newEntry->name == 0) {
		free (newEntry);
		return 1; //не удалось присвоить имя 
	}
	newEntry->value = value;
	newEntry->next = nameTable;
	nameTable = newEntry;
	return 0; // метка успешно добавлена в таблицу символов
}

int nameTableSearch (const char *name, uint16_t *valueDst) {
	struct entry *iterator = nameTable;
	while (iterator != 0) {
		if (0 == strcmp (iterator->name, name)) {
			if (valueDst)
				*valueDst = iterator->value;
			return 0; //метка найдена 
		}
		iterator = iterator->next;
	}
	return 1; //метка не найдена
}

uint16_t getAdress (const char *name) {
	uint16_t ans;
	struct entry *iterator = nameTable;
	while (iterator != 0) {
		if (0 == strcmp (iterator->name, name)) {
			return (iterator -> value) * 2; //метка найдена 
		}
		iterator = iterator->next;
	}
	return 1; //метка не найдена
}
int printTable () {
	struct entry *iterator = nameTable;
	while (iterator != 0) {
		printf("name:%s\norg:%d\n\n", iterator->name, iterator->value);
		iterator = iterator->next;
	}
	return 0;
}

