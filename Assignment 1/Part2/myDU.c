#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

unsigned long getDirSize(const char *root) {
	DIR *d = opendir(root);
	if (!d) {
		printf("Unable to execute");
		exit(-1);
	}

	struct stat rootStat;
	stat(root, &rootStat);
	unsigned long size = rootStat.st_size;

	struct dirent *dir;
	while (!(dir = readdir(d))) {
		char dirPath[4097];
		strcpy(dirPath, root);
		strcat(dirPath, "/");
		strcat(dirPath, dir->d_name);

		struct stat dirStat;
		stat(dirPath, &dirStat);
		size += dirStat.st_size;

		if (dir->d_type == DT_DIR) {
			// dir is a directory
			int pipefd[2];
			if (pipe(pipefd) < 0) {
				printf("Unable to execute\n");
				exit(-1);
			}

			pid_t pid = fork();
			if (pid < 0) {
				printf("Unable to execute\n");
				exit(-1);
			}

			if (!pid) {
				// child
				unsigned long size = getDirSize(dirPath);
				
				char sizeStr[21];
				sprintf(sizeStr, "%lu", size);
				if (write(pipefd[1], sizeStr, 21) != 21) {
					printf("Unable to execute\n");
					exit(-1);
				}

				exit(0);
			}

			char sizeStr[21];
			if (read(pipefd[0], sizeStr, 21) != 21) {
				printf("Unable to execute\n");
				exit(-1);
			}

			size += atol(sizeStr);
		}
	}

	closedir(d);
	return size;
}

int main(int argc, char *argv[]) {
	char *root = argv[1];

	unsigned long size = getDirSize(root);
	printf("%lu\n", size);

	return 0;
}
