#include "Files.c"

void initRefs()
{
    if (!file_exists(".refs"))
    {
        system("mkdir .refs");
        system("touch .refs/master");
        system("touch .refs/HEAD");
    }
}

void createUpdateRef(char *ref_name, char *hash)
{
    char buff[100];
    sprintf(buff, "echo %s >.refs/%s", hash, ref_name);
    system(buff);
}

void deleteRef(char *ref_name)
{
    char buff[256];
    sprintf(buff, ".refs/%s", ref_name);
    if (!file_exists(buff))
    {
        printf("The reference %s does not exist", ref_name);
    }
    else
    {
        sprintf(buff, "rm .refs/%s", ref_name);
    }
    system(buff);
}

char *getRef(char *ref_name)
{
    FILE *fp;
    char *result = malloc(sizeof(char) * 256);
    char buff[256];
    sprintf(buff, ".refs/%s", ref_name);
    if (!file_exists_in_dir(".refs", ref_name))
    {
        printf("The reference %s does not exist", ref_name);
        return NULL;
    }
    fp = fopen(buff, "r");
    if (fp == NULL)
    {
        printf("Error opening file. \n");
        return NULL;
    }
    fgets(result, 256, fp);
    result[strcspn(result, "\n")] = 0;
    fclose(fp);
    return result;
}