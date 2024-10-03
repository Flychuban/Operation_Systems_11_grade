#include <stdio.h>
#include <sys/stat.h>

int main()
{
    struct stat fileStat;
    if (stat("example.txt", &fileStat) < 0)
    {
        perror("Error retrieving file attributes");
        return 1;
    }

    printf("File size: %ld bytes\n", fileStat.st_size);
    printf("Last modified: %ld\n", fileStat.st_mtime);
    printf("File permissions: %o\n", fileStat.st_mode & 0777);

    return 0;
}