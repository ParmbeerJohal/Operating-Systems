For this assignment, I used the block and superblock struct data structures to store relevant information.

For diskinfo.c, I simply copied the information from the test.img file using the memcpy() function and printed out the information,
while making sure the information was printed in the correct order. The mmap() function made sure that the logical file system was
accessed in the correct location using the htonl commands.
After setting the pointer, I stored the start and end positions for the file allocation table
and ran a while loop through it to obtain the required information.

For disklist.c, my main method was similar to diskinfo.c except for printing out the list.
This was done by iterating through block contents (being divided into 64 parts)
and using bitwise operations.

For diskget.c, I again created the block and fat information variables, as well as
the mapped memory pointer.
In this case, the iteration was through each file, checking to see if it was the one
that was asked for. If found, the flag is set to 1 and the file is copied from the file
system into the cwd. 

**********NOTE: It also works with sub directories.**********

diskput.c was not finished due to personal time constraints and so has been excluded from
the submission and the makefile.
