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
 * Program: diskget.c
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

void checkStat(char stat, int size, void *file, int start, int jump, int index, char *temp) {
	if(stat & (1 << 1)){
	} else if(stat & (1 << 2)){
        }
	memcpy(&size, file + start + jump + 9, 4);
        size = ntohl(size);
        for(int count = 27; count < 54; count++){
        	memcpy(&stat, file + start + jump + count, 1);
                temp[index] = stat;
                index++;
	}

}

int main(int argc, char* argv[]) {

    	int fd = open(argv[1], O_RDWR);

    	struct stat buffer;
    	if (fstat(fd, &buffer) < 0) {
                fprintf(stderr, "file status error.");
                exit(1);
        }
    	char get_file[1024];

	strcpy(get_file, argv[3]);

    	void *address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    	struct superblock_t *sb;
    	sb = (struct superblock_t *)address;
    
    	int block_size = htons(sb->block_size);
    	int block_count = htonl(sb->file_system_block_count);

	int root_block_count = htonl(sb->root_dir_block_count);
	int root_start_block = htonl(sb->root_dir_start_block);
    
    	int fat_start_block = htonl(sb->fat_start_block);
    	int start_byte = root_start_block * block_size;
    	int fat_start_byte = fat_start_block * block_size;
   	void *file = mmap(NULL, block_count * block_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    	int fssize;
	char temp[1024];
	int jump = 0;
	char stat;   
	int index = 0;
	int flag = 0;


    	for(int i = 0; i < root_block_count * (block_size / 64); i++){
    		memcpy(&stat, file + start_byte + jump, 1);
		
    		if(stat & (1 << 0)){
			checkStat(stat, fssize, file, start_byte, jump, index, temp);
			
			if(strcmp(temp, get_file) == 0){
				flag = 1;
    			
    				int file_write = open(argv[3], O_TRUNC | O_CREAT | O_WRONLY | O_RDONLY | O_APPEND, 777);
    			
				memcpy(&fssize, file + start_byte + jump + 5, 4);
    				fssize=ntohl(fssize);
    			
				memcpy(&fssize, file + start_byte + jump + 9, 4);
    				fssize=ntohl(fssize);
    				
				int file_size = fssize;
    			
    				memcpy(&fssize, file + start_byte + jump + 1, 4);
    				fssize = ntohl(fssize);
    			
				int size_left = file_size;
				char buf;
				while(fssize != 0xFFFFFFFF) {

					if(size_left > block_size) {
						size_left = size_left - block_size;
						for(int i = 0; i < block_size; i++){
							memcpy(&buf, file + block_size * fssize + i, 1);
							write(file_write, &buf, 1);
						}

					} else {
						for(int i = 0; i < size_left; i++){
							memcpy(&buf, file + block_size * fssize + i, 1);
							write(file_write, &buf, 1);
						}	
					}
					memcpy(&fssize, file + fat_start_byte + fssize * 4, 4);
					fssize = ntohl(fssize);
				}

			}
			index = 0;
		}
		jump += 64;
    	}
	if(flag == 0)
		printf("File not found.\n");

    	munmap(address,buffer.st_size);
    	close(fd);

	return 1;
}
