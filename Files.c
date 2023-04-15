#include "List.c"
#include "permissions.c"
#include "Hash.c"

List *listdir(char *root_dir)
{
    DIR *dp;
    struct dirent *ep;
    List *L = initList();
    *L = NULL;
    Cell *temp_cell;
    dp = opendir(root_dir);

    // On parcourt la liste des repertoires et fichiers dans root_dir et
    // on les ajoute a notre liste
    if (dp != NULL)
    {
        while ((ep = readdir(dp)) != NULL)
        {
            temp_cell = buildCell(ep->d_name);
            insertFirst(L, temp_cell);
            List ptr = *L;
            while (ptr != NULL)
            {
                ptr = ptr->next;
            }
        }
        (void)closedir(dp);
    }
    else
    {
        perror("Error: Could not open the directory");
        return NULL;
    }

    return L;
}

int file_exists_in_dir(char *dir, char *file)
{
    List *ptr = listdir(dir);
    List ptrFile = searchList(ptr, file);

    if (ptrFile == NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

struct stat st = {0};

int file_exists(char *file)
{
    struct stat buffer;
    if (stat(file, &buffer) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void cp(char *to, char *from)
{
    // On recupere le contenu de from (path forme a partir du hash)
    // puis on le copie dans to
    if (file_exists(from))
    {
        char ligne[256];
        FILE *ficfrom = fopen(from, "r");
        FILE *ficto = fopen(to, "w");

        while (fgets(ligne, 256, ficfrom) != NULL)
        {
            fputs(ligne, ficto);
        }
        fclose(ficto);
        fclose(ficfrom);
    }
}

char *hashToPath(char *hash)
{
    // A partir des deux premiers characteres du hash, on cree un repertoire ou
    // on enregistrera le fichier contenant le contenu commit / wt / fichier.
    // Ce fichier sera nomme a partir du reste des characteres du hash
    int hash_size = strlen(hash);
    char *dir = malloc((hash_size + 1) * sizeof(char));
    dir[0] = hash[0];
    dir[1] = hash[1];
    dir[2] = *"/";
    int i;
    for (i = 3; i <= hash_size; i++)
    {
        dir[i] = hash[i - 1];
    }
    dir[i] = *"\0";
    return dir;
}

void blobFile(char *file)
{
    char *hash = sha256file(file);
    char *ch2 = strdup(hash);
    ch2[2] = *"\0";
    if (!file_exists(ch2))
    {
        char buff[100];
        sprintf(buff, "mkdir %s", ch2);
        system(buff);
    }
    char *ch = hashToPath(hash);
    cp(ch, file);
}

char *hashToFile(char *hash)
{
    char *ch2 = strdup(hash);
    ch2[2] = *"\0";
    struct stat st;
    if (stat(ch2, &st) == -1)
    {
        mkdir(ch2, 0700);
    }
    return hashToPath(hash);
}

char *concat_paths(char *path1, char *path2)
{
    // Ajoute path2 a path1
    char *result = malloc(strlen(path1) + strlen(path2) + 1);
    if (result == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        return NULL;
    }
    strcpy(result, path1); // Copy path1 to result
    strcat(result, "/");   // Append a slash to result
    strcat(result, path2); // Append path2 to result
    return result;
}

void createFile(char *file)
{
    char buff[100];
    sprintf(buff, "touch %s", file);
    system(buff);
}
