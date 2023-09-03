#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	unsigned long inputNum = atoi(argv[argc - 1]);
	unsigned long result = inputNum * inputNum;

	if (argc == 2) {
		printf("%lu\n", result);
		return 0;
	}

	char *cargv[argc];
	cargv[0] = argv[1];
	for (int i = 1; i < argc - 2; i++) {
		cargv[i] = argv[i + 1];
	}
	char resultStr[21];
	sprintf(resultStr, "%lu", result);
	cargv[argc - 2] = resultStr;
	cargv[argc - 1] = NULL;

	char cpath[9] = "./";
	if (execv(strcat(cpath, cargv[0]), cargv)) {
		printf("Unable to execute\n");
		exit(-1);
	}

	return 0;
}
