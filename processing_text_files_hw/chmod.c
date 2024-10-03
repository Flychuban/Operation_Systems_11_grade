#include <stdio.h>
#include <sys/stat.h>

int main()
{
    if (chmod("example.txt", 0644) < 0)
    {
        perror("Error changing file permissions");
        return 1;
    }

    printf("Permissions changed successfully\n");
    return 0;
}