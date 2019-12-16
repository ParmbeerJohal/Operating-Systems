#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>

/*********
 * Parm Johal
 * Student id: V00787710
 * CSC 360
 * Assignment p3
 * Program: disklist.c
 * -----------------------------------------------------------
 * REFERENCES
 * http://man7.org/linux/man-pages/man2/mmap.2.html
 * https://linux.die.net/man/2/fstat
 * http://www.c4learn.com/c-programming/c-bitwise-right-shift/
 * CSC 360 tutorial 9 & X slides
 * Lecture slides courtesy of Prof. Pan
 * Consulted https://github.com/kaoaaron/File-System/blob/master/
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
    uint8_t   fs_id [8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

void printList(int start, char *file, int root_block_count, int block_size) {
	int fssize;
        char stat;
        int jump = 0;
        int spacing = 0;

    for(int i = 0; i < root_block_count * (block_size / 64); i++){
        memcpy(&stat, file + start + jump, 1);
	 if(stat & (1 << 0)){
                if(stat & (1 << 1)){
                        printf("F ");
                }
                else if(stat & (1 << 2)){
                        printf("D ");
                }
                memcpy(&fssize, file + start + jump + 9, 4);
                fssize = ntohl(fssize);
                printf("%10d ", fssize);
                for(int name_count = 27; name_count < 54; name_count++){
                        memcpy(&stat, file + start + jump + name_count, 1);
                        if(spacing == 0){
                                printf("%30c", stat);
                                spacing = 1;
                        }
                        else if(spacing == 1){
                                printf("%c", stat);
                        }
                }
                memcpy(&fssize, file + start + jump + 20, 2);
                fssize = ntohs(fssize);
                printf("\t%d/", fssize);
                memcpy(&stat, file + start + jump + 22, 1);
                printf("%.2d/", stat);
                memcpy(&stat, file + start + jump + 23, 1);
                printf("%.2d ", stat);
                memcpy(&stat, file + start + jump + 24, 1);
                printf("%.2d:", stat);
                memcpy(&stat, file + start + jump + 25, 1);
                printf("%.2d:", stat);
                memcpy(&stat, file + start + jump + 26, 1);
                printf("%.2d", stat);
                printf("\n");

        }
	spacing = 0;
	jump += 64;
    }


}

int main(int argc, char* argv[]) {

	int fd = open(argv[1], O_RDWR);
    	struct stat buffer;
	if (fstat(fd, &buffer) < 0) {
        	fprintf(stderr, "file status error.");
                exit(1);
        }

    	void *address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    	struct superblock_t* sb;
    	sb = (struct superblock_t *)address;
    
    	int block_size = htons(sb->block_size);
    	int block_count = htonl(sb->file_system_block_count);
	int root_block_count = htonl(sb->root_dir_block_count);
	int root_start_block = htonl(sb->root_dir_start_block);
    
    	int start = root_start_block * block_size;
    
    	void *file = mmap(NULL, block_count * block_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


	printList(start, file, root_block_count, block_size);
	

    	munmap(address,buffer.st_size);
    	close(fd);

	return 1;
}
