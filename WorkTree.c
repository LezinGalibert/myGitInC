#include <stdlib.h>
#include "Ref.c"

typedef struct
{
    char *name;
    char *hash;
    int mode;
} WorkFile;

typedef struct
{
    WorkFile *tab;
    int size;
    int n;
} WorkTree;

WorkFile *createWorkFile(char *name)
{
    WorkFile *wf = malloc(sizeof(WorkFile));
    wf->name = strdup(name);
    wf->hash = NULL;
    wf->mode = 0;
    return wf;
}

char *wfts(WorkFile *wf)
{
    char *ch = malloc(1000 * sizeof(char));
    sprintf(ch, "%s\t%s\t%d", wf->name, wf->hash, wf->mode);
    return ch;
}

WorkFile *stwf(char *ch)
{
    char *name = malloc(sizeof(char) * 1000);
    char *hash = malloc(sizeof(char) * 1000);
    int mode;
    sscanf(ch, "%s\t%s\t%d", name, hash, &mode);
    WorkFile *wf = createWorkFile(name);
    wf->mode = mode;
    wf->hash = hash;
    return wf;
}

WorkTree *initWorkTree()
{
    WorkTree *wt = malloc(sizeof(WorkTree));
    wt->tab = malloc(N * sizeof(WorkFile));
    wt->size = N;
    wt->n = 0;
    return wt;
}

int inWorkTree(WorkTree *wt, char *name)
{
    for (int i = 0; i < wt->n; i++)
    {
        if (strcmp(wt->tab[i].name, name) == 0)
            return i;
    }
    return -1;
}

int appendWorkTree(WorkTree *wt, char *name, char *hash, int mode)
{
    if (inWorkTree(wt, name) >= 0)
    {
        printf("File <%s> is already in the worktree \n", name);
        return -4;
    }
    if (wt->size > wt->n)
    {
        wt->tab[wt->n].mode = mode;
        wt->tab[wt->n].name = strdup(name);
        if (hash == NULL)
            wt->tab[wt->n++].hash = NULL;

        else
        {
            wt->tab[wt->n++].hash = strdup(hash);
        }
    }
    return 0;
}

char *wtts(WorkTree *wt)
{
    char *ch = malloc(wt->size * sizeof(char) * 1000);
    int len = 0;
    for (int i = 0; i < wt->n; i++)
    {
        char *ch2 = wfts(wt->tab + i);
        strcat(ch, ch2);
        strcat(ch, "\n");
    }
    return ch;
}

WorkTree *stwt(char *ch)
{
    int pos = 0;
    int n_pos = 0;
    int sep = "\n";
    char *ptr;
    char *result = malloc(sizeof(char) * 10000);
    WorkTree *wt = initWorkTree();
    while (pos < strlen(ch))
    {
        ptr = strchr(ch + pos, sep);
        if (ptr == NULL)
            n_pos = strlen(ch) + 1;
        else
        {
            n_pos = ptr - ch + 1;
        }
        memcpy(result, ch + pos, n_pos - pos - 1);
        result[n_pos - pos - 1] = *"\0";
        pos = n_pos;
        WorkFile *wf = stwf(result);
        appendWorkTree(wt, wf->name, wf->hash, wf->mode);
    }
    return wt;
}

int wttf(WorkTree *wt, char *path)
{
    FILE *fp = fopen(path, "w");
    if (fp != NULL)
    {
        fputs(wtts(wt), fp);
        fclose(fp);
        return 0;
    }
    return -1;
}

WorkTree *ftwt(char *file)
{
    char *buff = malloc(sizeof(char) * N);
    FILE *f = fopen(file, "r");
    char *all_wf = malloc(sizeof(char) * N * MAX_FILES);
    while (fgets(buff, N, f) != NULL)
    {
        strcat(all_wf, buff);
    }
    return stwt(all_wf);
}

char *blobWorkTree(WorkTree *wt)
{
    char fname[100] = "tmp/myfileXXXXXX";
    int fd = mkstemp(fname);
    wttf(wt, fname);
    char *hash = sha256file(fname);
    char *ch = hashToFile(hash);
    strcat(ch, ".t");
    cp(ch, fname);
    return hash;
}

char *saveWorkTree(WorkTree *wt, char *path)
{
    for (int i = 0; i < wt->n; i++)
    {
        char *absPath;
        if (strcmp(path, ".") != 0)
        {
            absPath = concat_paths(path, wt->tab[i].name);
        }
        else
        {
            absPath = wt->tab[i].name;
        }
        if (file_exists(wt->tab[i].name) == 1)
        {
            blobFile(absPath);
            wt->tab[i].hash = sha256file(absPath);
            wt->tab[i].mode = getChmod(absPath);
        }
        else
        {
            WorkTree *wt2 = initWorkTree();
            List *L = listdir(absPath);
            for (Cell *ptr = *L; ptr != NULL; ptr = ptr->next)
            {
                if (strcmp(ptr->data[0], *"."))
                    continue;
                appendWorkTree(wt2, ptr->data, 0, NULL);
            }
            wt->tab[i].hash = saveWorkTree(wt2, absPath);
            wt->tab[i].mode = getChmod(absPath);
        }
    }
    return blobWorkTree(wt);
}

int isWorkTree(char *hash)
{
    if (file_exists(strcat(hashToPath(hash), ".t")))
    {
        return 1;
    }
    if (file_exists(hashToPath(hash)))
    {
        return 0;
    }
    return -1;
}

void restoreWorkTree(WorkTree *wt, char *path)
{
    for (int i = 0; i < wt->n; i++)
    {
        char *absPath;
        if (strcmp(path, ".") != 0)
        {
            absPath = concat_paths(path, wt->tab[i].name);
        }
        else
        {
            absPath = wt->tab[i].name;
        }
        char *copyPath = hashToPath(wt->tab[i].hash);
        char *hash = wt->tab[i].hash;
        if (isWorkTree(hash) == 0)
        { // si c’est un fichier
            cp(absPath, copyPath);
            setMode(getChmod(copyPath), absPath);
        }
        else
        {
            if (isWorkTree(hash) == 1)
            { // si c’est un repertoire
                strcat(copyPath, ".t");
                WorkTree *nwt = ftwt(copyPath);
                restoreWorkTree(nwt, absPath);
                setMode(getChmod(copyPath), absPath);
            }
        }
    }
}

WorkTree *mergeWorkTrees(WorkTree *wt1, WorkTree *wt2, List *conflicts)
{
    WorkTree *noConflictWT = initWorkTree();
    WorkTree *tmp;

    if (wt1->n < wt2->n)
    {
        tmp = wt1;
        wt1 = wt2;
        wt2 = tmp;
    }

    for (int i = 0; i < wt1->n; i++)
    {
        if (inWorkTree(wt2, wt1->tab[i].name) == 0)
        {
            Cell *cell = buildCell(wt1->tab[i].name);
            insertFirst(conflicts, cell);
        }
        else
        {
            appendWorkTree(noConflictWT, wt1->tab[i].name, wt1->tab[i].hash, wt1->tab[i].mode);
        }
    }
    // while (conflicts != NULL)
    // {
    //     char *val = (conflicts)->data;
    //     (conflicts)->next;
    // }

    return noConflictWT;
}
