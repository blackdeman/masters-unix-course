#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	
	int sizeInTb = 100;
	
	if (argc == 2) {
		sizeInTb = atoi(argv[1]);
	}

	off_t size = (unsigned long)sizeInTb << 40;

	void* p = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (p == MAP_FAILED) {
       		perror("mmap");
		return 1;
	}

	while(1);
	getchar();

	if (munmap(p, size) == -1) {
		perror("munmap");
		return 1;
	}
}
