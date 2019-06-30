#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

struct mark {
	char *name;
	size_t location;
};

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
	//int pass = 1;
	//FILE *input = fopen(nameInput, "rt");
	//FILE *output = fopen (nameOutput, "wb");
	// process(1, input, output);
	return 0;
}

int process (int pass, FILE *input, FILE *output) {
	int org = 31;
	char lineBuffer [1024];
	while (!feof(input)) {
		if (0 == fgets (lineBuffer, sizeof(lineBuffer), input))
			return 2; // ошибка чтения
		int result = processLine(output, org, lineBuffer);
		if (result != 0)
			return result; 
	}
}
int processLine (FILE *output, int *org, char *line) {
	return 0;
}