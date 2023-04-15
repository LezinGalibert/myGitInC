#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hashFile(char *source, char *dest)
{
    char buff[100];
    sprintf(buff, "cat %s | sha256sum > %s", source, dest);
    system(buff);
    return 0;
}

char *sha256file(char *file)
{
    static char template[] = "tmp/myfileXXXXXX";
    char fname[1000];
    strcpy(fname, template);
    int fd = mkstemp(fname);

    char cmd[20000];
    sprintf(cmd, "cat %s | sha256sum > %s", file, fname);
    system(cmd);

    char *hash = malloc(65 * sizeof(char));
    FILE *f = fopen(fname, "r");
    fgets(hash, 65, f);
    hash[64] = *"\0";

    return hash;
}

unsigned long hashF(char *str)
{
    // Method DJB2
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}