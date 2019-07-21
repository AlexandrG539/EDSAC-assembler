#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <stdint.h>
#include "nameTable.h"
#include "bitVector.h"

struct label {
	char *name;
	size_t location;
};

regex_t label;
regex_t command;
regex_t format;
regex_t formatDirective;
regex_t directiveArray;
regex_t directiveConst;
regex_t directiveOrg;
regmatch_t sub[6];

int strNum = 1;
uint32_t memory[993]; // образ памяти 
uint16_t orgMax = 31;

int process (int pass, FILE *input);
int processLine (uint16_t *org, char *line, int linenum, int pass);
int printAns(FILE *output);
int readNum (char *array);
int deleteLast(char *line);

int main (int argc, char **argv) {
	create (993); // битовый массив отображает, происходила ли запись в ячейку
	char *nameInput;
	char *nameOutput;
	if (argc <= 1) {
         printf("syntax : \n\t-i name of imput file\n\t-o name of output file\n", argv[0]);
         return 6; //код ошибки 6 - ошибка, связанная с командной строкой
     }
	if (argc != 5) {
		printf("Parameters entered incorrectly");
		return 6;
	}
	if ((!strcmp(argv[1], "-i") && !strcmp (argv[3], "-o")) 
		|| (!strcmp(argv[3], "-i") && !strcmp (argv[1], "-o"))) {
		if (!strcmp(argv[1], "-i")) {
			nameInput = argv[2];
			nameOutput = argv[4];
		}
		else if (!strcmp(argv[1], "-o")) {
			nameInput = argv[4];
			nameOutput = argv[2];
		}
	} 
	else {
		printf ("Wrong pare of flags:'%s', '%s', required '-i' and '-o'\n", argv[1], argv[3]);
		return 6;
	}
	FILE *input = fopen(nameInput, "rt");
	FILE *output = fopen (nameOutput, "wb");
	if(regcomp(&label, "^[[:space:]]*([[:alpha:]][[:alnum:]]*):", REG_EXTENDED)) {
		printf ("Could not compile regex label\n");
		return 7;
	}
	if(regcomp(&command, "[[:space:]]*([ASHVNTUCRLEGIOFXYZ])[[:space:]]+([[:alpha:]][[:alnum:]]*)", REG_EXTENDED)) {
		printf ("Could not compile regex command\n");
		return 7;
	}
	if (regcomp(&format,"^[[:space:]]*([[:alpha:]][[:alnum:]]*:)?[[:space:]]*([ASHVNTUCRLEGIOFXYZ][[:space:]]+[[:alpha:]][[:alnum:]]*)?[[:space:]]*(\\/\\/[[:print:]]*)?$", REG_EXTENDED)) {
		printf ("Could not compile regex format\n");
		return 7;
	}
	if (regcomp(&directiveArray,"^[[:space:]]*([[:alpha:]][[:alnum:]]*:)?[[:space:]]*.array[[:space:]]+(([[:digit:]]+)|(0x[[:xdigit:]]+))[[:space:]]*(\\/\\/[[:print:]]*)?$", REG_EXTENDED)) {
		printf ("Could not compile regex directiveArray\n");
		return 7;
	}
	if (regcomp(&directiveConst,"^[[:space:]]*([[:alpha:]][[:alnum:]]*:)?[[:space:]]*.const[[:space:]]+(([[:digit:]]+)|(0x[[:xdigit:]]+))[[:space:]]*(\\/\\/[[:print:]]*)?$", REG_EXTENDED)) {
		printf ("Could not compile regex directiveConst\n");
		return 7;
	}
	if (regcomp(&directiveOrg,"^[[:space:]]*([[:alpha:]][[:alnum:]]*:)?[[:space:]]*.org[[:space:]]+(([[:digit:]]+)|(0x[[:xdigit:]]+))[[:space:]]*(\\/\\/[[:print:]]*)?$", REG_EXTENDED)) {
		printf ("Could not compile regex directiveOrg\n");
		return 7;
	}
	int result = process(1, input);
	if (result != 0)
		return result;
	rewind(input);
	process (2, input);
	if (result != 0)
		return result;
	printAns(output);
	return 0;
}

int process (int pass, FILE *input) {
    uint16_t org = 31;
    int linenum = 1;
	char lineBuffer [1024];
	while (!feof(input)) {
        if (0 == fgets (lineBuffer, sizeof(lineBuffer), input)) {
            printf ("Reading error");
			return 2; // ошибка чтения
        }
		deleteLast (lineBuffer);
		int result = processLine(&org, lineBuffer, linenum, pass);
		if (result != 0) {
			if (result == 1) {
				printf ("String %d has wrong format\nDEBUG %d\n", linenum, result);
				return result;
			}
			if (result == 2) {
				printf ("Error in string %d: There are two identical labels.\nDEBUG %d\n", linenum, result);
				return result;
			}
			if (result == 3) {
				printf ("Error in string %d: memory cell full\nDEBUG %d\n", linenum, result);
				return result;
			}
			if (result == 4) {
				printf ("Error in string %d: out of memory\nDEBUG %d\n", linenum, result);
				return result;
			}
			if (result == 5) {
				printf ("Error in string %d: label not declared\nDEBUG %d\n", linenum, result);
				return result;
			}
            printf ("String %d has wrong format\nDEBUG %d\n", linenum, result);
            return result;
        }
        linenum++;
	}
    return 0;
}

int processLine (uint16_t *org, char *line, int linenum, int pass) {
	if(*org < 31 || *org > 1024) 
		return 4;
	if ((regexec(&format, line, 0, NULL, 0) != 0) && (regexec(&directiveArray, line, 0, NULL, 0) != 0) && (regexec(&directiveConst, line, 0, NULL, 0) != 0) &&(regexec(&directiveOrg, line, 0, NULL, 0) != 0)) {
			return 1;
		}
	if (pass == 1) {
		if (0 == regexec(&label, line, 6, sub, 0)) {
			char labelTxt[1024];
			size_t len = sub[1].rm_eo - sub[1].rm_so;
			memcpy (labelTxt, &(line [sub[1].rm_so]), len);
			labelTxt[len] = 0;
			if (nameTableAdd (labelTxt,*org) != 0)
			return 2; // две метки с одинаковым именем
		}
	}
	if (0 == regexec(&command, line, 6, sub, 0)) {
		if (pass == 1) {
			*org = *org + 1;
			if(*org < 31 || *org > 1024)
				return 4;
		}
		if (pass == 2) {
			char cmd[2];
			memcpy (cmd, &(line [sub[1].rm_so]), 1);
			cmd[1] = 0;
			char labelName[1024];
			size_t l = sub[2].rm_eo - sub[2].rm_so;
			memcpy (labelName, &(line [sub[2].rm_so]), l);
			labelName[l] = 0;
			if (getAdress(labelName) == 1)
				return 5;
			if (get(*org - 31))
				return 3;
			if (cmd[0] == 'A') {
				memory[*org - 31] = 114688 + getAdress(labelName);
			}				
			else if (cmd[0] == 'S') {
				memory[*org - 31] = 49152 + getAdress(labelName);
			}
			else if (cmd[0] == 'H') {
				memory[*org - 31] = 86016 + getAdress(labelName);
			}
			else if (cmd[0] == 'V') {
				memory[*org - 31] = 126976 + getAdress(labelName);
			}
			else if (cmd[0] == 'N') {
				memory[*org - 31] = 90112 + getAdress(labelName);
			}
			else if (cmd[0] == 'T') {
				memory[*org - 31] = 20480 + getAdress(labelName);
			}
			else if (cmd[0] == 'U') {
				memory[*org - 31] = 28672 + getAdress(labelName);
			}
			else if (cmd[0] == 'C') {
				memory[*org - 31] = 122880 + getAdress(labelName);
			}
			else if (cmd[0] == 'R') {
				memory[*org - 31] = 16384 + getAdress(labelName);
			}
			else if (cmd[0] == 'L') {
				memory[*org - 31] = 102400 + getAdress(labelName);
			}
			else if (cmd[0] == 'E') {
				memory[*org - 31] = 12288 + getAdress(labelName);
			}
			else if (cmd[0] == 'G') {
				memory[*org - 31] = 110592 + getAdress(labelName);
			}
			else if (cmd[0] == 'I') {
				memory[*org - 31] = 32768 + getAdress(labelName);
			}
			else if (cmd[0] == 'O') {
				memory[*org - 31] = 36864 + getAdress(labelName);
			}
			else if (cmd[0] == 'F') {
				memory[*org - 31] = 69632 + getAdress(labelName);
			}
			else if (cmd[0] == 'X') {
				memory[*org - 31] = 106496 + getAdress(labelName);
			}
			else if (cmd[0] == 'Y') {
				memory[*org - 31] = 24576 + getAdress(labelName);
			}
			else if (cmd[0] == 'Z') {
				memory[*org - 31] = 53248;
			}
			set(*org - 31, 1);
			*org = *org + 1;
			if (*org > orgMax)
				orgMax = *org;
			return 0;
		}
	}
	if (0 == regexec(&directiveArray, line, 6, sub, 0)) {
		char arrayCh[5];
		size_t a = sub[2].rm_eo - sub[2].rm_so;
		memcpy (arrayCh, &(line [sub[2].rm_so]), a);
		arrayCh[a] = 0;
		int arrayInt = readNum(arrayCh);
		if (pass == 1) {
			*org = *org + arrayInt - 1;
			if(*org < 31 || *org > 1024)
				return 4;
		}
		if (pass == 2) {
			while (arrayInt != 0) { 
				if (get(*org - 31)) {
					return 3;
				}
				else {
					set (*org - 31, 1);
					*org = *org + 1;
					arrayInt--;
					if(*org < 31 || *org > 1024)
						return 4;
				}
			}
		if (*org > orgMax)
				orgMax = *org;
		}
		return 0;
	}
	else if (0 == regexec(&directiveConst, line, 6, sub, 0)) {
		char constCh[6];
		size_t c = sub[2].rm_eo - sub[2].rm_so;
		memcpy (constCh, &(line [sub[2].rm_so]), c);
		constCh[c] = 0;
		int constInt = readNum(constCh);
		if (pass == 1) {
			*org = *org + 1;
			if(*org < 31 || *org > 1024)
				return 4;
			return 0;
		}
		if (!get(*org - 31)) { 
			memory[*org - 31] = constInt;
			set (*org - 31, 1);
			*org = *org + 1;
			if (*org > orgMax)
				orgMax = *org;
			return 0;
		}
		else return 3;
	}
	else if (0 == regexec(&directiveOrg, line, 6, sub, 0)) {
		char orgCh[5];
		size_t o = sub[2].rm_eo - sub[2].rm_so;
		memcpy (orgCh, &(line [sub[2].rm_so]), o);
		orgCh[o] = 0;
		int orgInt = readNum(orgCh);
		*org = orgInt;
		if (*org > orgMax)
				orgMax = *org;
		if(*org < 31 || *org > 1024)
			return 4;
		return 0;
	}
	else return 0;
}

int readNum (char *array) {
	char *end;
	int ans = strtol(array, &end, 0);
	return ans;
}

int printAns(FILE *output) {
	for (int i = 0; i <= orgMax - 32; i++) {
		fprintf(output, "cell%d:%05lX\n", i + 31, memory[i]);
	}
	return 0;
}
int deleteLast(char *line) {
	char *pos;
	if ((pos=strchr(line, '\n')) != NULL) 
		*pos = '\0';
    if ((pos=strchr(line, '\r')) != NULL) 
		*pos = '\0';
	return 0;
}
