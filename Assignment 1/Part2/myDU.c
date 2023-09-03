#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

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

		if (dir->d_type == DT_REG) {
			// dir is a file
			struct stat dirStat;
			stat(dirPath, &dirStat);
			size += dirStat.st_size;
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
