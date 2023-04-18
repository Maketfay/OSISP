#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

struct record_s {
    char name[80];
    char address[80];
    char semester;
};


int main()
{
    struct record_s arrayRecords[10];
    for(int i = 0; i< 10; i++)
    {
        sprintf(arrayRecords[i].name, "Name_%d", i);
        sprintf(arrayRecords[i].address, "Address%d", i);
        arrayRecords[i].semester = i; 
    }
    int fd = open("records.bin", O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    write(fd, &arrayRecords, sizeof(arrayRecords));
    return 0;
}