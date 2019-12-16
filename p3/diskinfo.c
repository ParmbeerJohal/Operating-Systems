#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

/*********
 * Parm Johal
 * Student id: V00787710
 * CSC 360
 * Assignment p3
 * Program: diskinfo.c
 * -----------------------------------------------
 * REFERENCES
 * http://man7.org/linux/man-pages/man2/mmap.2.html
 * https://linux.die.net/man/2/fstat
 * http://www.c4learn.com/c-programming/c-bitwise-right-shift/
 * CSC 360 tutorial 9 & X slides
 * Lecture slides courtesy of Prof. Pan
 * Consulted https://github.com/kaoaaron/File-System/blob/master/

 *
 *********/

typedef struct Block_info {
        int start_block;
        int end_block;
        char *sub_name;
        char *address;
} block;

struct __attribute__((__packed__)) dir_entry_timedate_t {
   uint16_t year;
   uint8_t month;
   uint8_t day;
   uint8_t hour;
   uint8_t min;
   uint8_t sec;
};

struct __attribute__((__packed__)) dir_entry_t {
   uint8_t status;
   uint32_t starting_block;
   uint32_t block_count;
   uint32_t size;
   struct dir_entry_timedate_t create_time;
   struct dir_entry_timedate_t modify_time;
   uint8_t filename[31];
   uint8_t unused[6];
};

struct __attribute__((__packed__)) superblock_t {
   uint8_t id[8];
   uint16_t size;
   uint32_t fsblk_count;
   uint32_t fatblk_start;
   uint32_t fatblk_count;
   uint32_t root_start;
   uint32_t root_count;
};


void printSuperBlock(struct superblock_t *sblock) {
	printf("Super block information:\n");
	
	printf("Block size: %d\n", htons(sblock->size));
	printf("Block count: %d\n", ntohl(sblock->fsblk_count));
	printf("FAT start: %d\n", ntohl(sblock->fatblk_start));
	printf("FAT blocks: %d\n", ntohl(sblock->fatblk_count));
	printf("Root directory start: %d\n", ntohl(sblock->root_start));
	printf("Root directory blocks: %d\n", ntohl(sblock->root_count));

}

void printFAT(int free, int reserved, int allocated) {
	printf("FAT information:\n");

	printf("Free Blocks: %d\n", free);
	printf("Reserved Blocks: %d\n", reserved);
	printf("Allocated Blocks: %d\n", allocated);

}



int main(int argc, char *argv[]) {
	int test_file = open(argv[1], O_RDWR);
	struct stat buffer;
	if (fstat(test_file, &buffer) < 0) {
        	fprintf(stderr, "file status error.");
        	exit(1);
	}

	void *map_ptr = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, test_file, 0);

	struct superblock_t *super_block = (struct superblock_t *)map_ptr;
	printSuperBlock(super_block);
	printf("\n");
	
	int fat_start = htons(super_block->size) * ntohl(super_block->fatblk_start);
	int fat_end = htons(super_block->size) * ntohl(super_block->fatblk_count);

	int free_blocks = 0;
	int reserved_blocks = 0;
	int allocated_blocks = 0;
	int i = fat_start;

	while(i < fat_start + fat_end) {
		int tracker = 0;
		memcpy(&tracker, map_ptr + i, 4);
		tracker = htonl(tracker);

		if (tracker == 1) {
			reserved_blocks++;
		} else if (tracker == 0) {
			free_blocks++;
		} else {
			allocated_blocks++;
		}

		i += 4;
	}

	printFAT(free_blocks, reserved_blocks, allocated_blocks);
	
	munmap(map_ptr, buffer.st_size);
	close(test_file);
	
	return 0;
}



