#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <stdint.h>
#include "nameTable.h"
struct label {
	char *name;
	size_t location;
};

regex_t label;
regex_t command;
regmatch_t sub[6];
int strNum = 1;

int process (int pass, FILE *input, FILE *output);
int processLine (FILE *output, uint16_t *org, char *line, int linenum, int pass);

int main (int argc, char **argv) {
	char *nameInput;
	char *nameOutput;
	if (argc <= 1) {
         printf("syntax : \n\t-i name of imput file\n\t-o name of output file\n", argv[0]);
         return 1; //код ошибки 1 - ошибка, связанная с командной строкой
     }
	if (argc != 5) {
		printf("Parameters entered incorrectly");
		return 1;
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
		return 1;
	}
	printf ("input: %s\n", nameInput);
	printf ("output: %s\n", nameOutput);
	FILE *input = fopen(nameInput, "rt");
	FILE *output = fopen (nameOutput, "wb");
	if(regcomp(&label, "^[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*):", REG_EXTENDED)) {
		printf ("Could not compile regex\n");
		return 3;
	}
	if(regcomp(&command, "^[[:space:]]*[ASHVNTUCRLEGIOFXYZ][[:space:]]*([[:alpha:]][[:alnum:]]*)", REG_EXTENDED)) {
		printf ("Could not compile regex\n");
		return 3;
	}
	process(1, input, output);
	rewind(input);
	return 0;
	//process (2, input, output);
	//return 0;
}

int process (int pass, FILE *input, FILE *output) {
    uint16_t org = 31;
    int linenum = 1;
	char lineBuffer [1024];
	while (!feof(input)) {
        if (0 == fgets (lineBuffer, sizeof(lineBuffer), input)) {
            printf ("DEBUG 1\n");
			return 2; // ошибка чтения
        }
		int result = processLine(output, &org, lineBuffer, linenum, pass);
		if (result != 0) {
            printf ("DEBUG 2\n");
            return result;
        }
        linenum++;
	}
	fprintf(output, "\n");
    return 0;
}
int processLine (FILE *output, uint16_t *org, char *line, int linenum, int pass) {
	if (0 == regexec(&command, line, 6, sub, 0)) {
		*org++;
	}
    if (0 == regexec(&label, line, 6, sub, 0)) {
    // Нашли
	for (int i = 0; i < 6; i++) {
	//	printf("rm_so %d = %d\n", i, sub[i].rm_so);
	//	printf("rm_eo %d = %d\n", i, sub[i].rm_eo);
	}
    char label_txt [1024];
    size_t len = sub [1].rm_eo - sub [1].rm_so;
    memcpy (label_txt, &(line [sub [1].rm_so]), len);
    label_txt [len] = 0;
	if (nameTableAdd (label_txt,*org) != 0)
		return 2;
    printf ("LABEL: %s, org: %d\n", label_txt, *org);
    }
    return 0;
}
