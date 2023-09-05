#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

unsigned long getSize(const char *root) {
	struct stat rootStat;
	if (stat(root, &rootStat) < 0) {
		printf("Unable to execute\n");
		exit(-1);
	}

	if (S_ISREG(rootStat.st_mode)) {
		// root is a file
		return rootStat.st_size;
	}

	if (S_ISDIR(rootStat.st_mode)) {
		// root is a directory
		unsigned long size = rootStat.st_size;

		DIR *d = opendir(root);
		if (!d) {
			printf("Unable to execute");
			exit(-1);
		}

		struct dirent *dir;
		while (dir = readdir(d)) {
			if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
				continue;
			}

			char dirPath[4097];
			strcpy(dirPath, root);
			strcat(dirPath, "/");
			strcat(dirPath, dir->d_name);

			size += getSize(dirPath);
		}

		closedir(d);
		return size;
	}
}

unsigned long getRootSize(const char *root) {
	struct stat rootStat;
	if (stat(root, &rootStat) < 0) {
		printf("Unable to execute\n");
		exit(-1);
	}

	unsigned long size = rootStat.st_size;

	DIR *d = opendir(root);
	if (!d) {
		printf("Unable to execute");
		exit(-1);
	}

	struct dirent *dir;
	while (dir = readdir(d)) {
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
			continue;
		}

		char dirPath[4097];
		strcpy(dirPath, root);
		strcat(dirPath, "/");
		strcat(dirPath, dir->d_name);

		if (dir->d_type == DT_REG || dir->d_type == DT_LNK) {
			// dir is a file or a symbolic link
			size += getSize(dirPath);
			continue;
		}

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
				unsigned long size = getSize(dirPath);
				
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
			continue;
		}
	}

	closedir(d);
	return size;
}

int main(int argc, char *argv[]) {
	char *root = argv[1];

	unsigned long size = getRootSize(root);
	printf("%lu\n", size);

	return 0;
}
