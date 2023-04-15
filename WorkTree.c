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
    // On parcourt les WF de WT et on cherche celui dont le nom correspond
    // a name
    for (int i = 0; i < wt->n; i++)
    {
        if (strcmp(wt->tab[i].name, name) == 0)
            return i;
    }
    return -1;
}

int appendWorkTree(WorkTree *wt, char *name, char *hash, int mode)
{
    // Si le fichier n'est pas deja dans le WT, on ajoute le WF correspondant
    // a la liste tab et on incremente la taille du tableau de 1
    if (inWorkTree(wt, name) >= 0)
    {
        printf("Error: File <%s> is already in the worktree \n", name);
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
    // Chaque WF etant stocke sous la form name\thash\tmode\n, on recupere chaque ligne en
    // trouvant l'indice de \n puis on forme un WF a partir du contenu de la ligne
    int increment = 0;
    int break_pos = 0;
    char *ptr;
    char *wf_content = malloc(sizeof(char) * 10000);
    WorkTree *wt = initWorkTree();
    while (increment < strlen(ch))
    {
        ptr = strchr(ch + increment, '\n');
        if (ptr == NULL)
            break_pos = strlen(ch) + 1;
        else
        {
            break_pos = ptr - ch + 1;
        }
        memcpy(wf_content, ch + increment, break_pos - increment - 1);
        wf_content[break_pos - increment - 1] = *"\0";
        increment = break_pos;
        WorkFile *wf = stwf(wf_content);
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
    // On parcourt le WT a sauvegarder, si on trouve un fichier,
    // on l'enregistre sinon on appelle la fonction recursivement
    // dans le repertoire
    for (int i = 0; i < wt->n; i++)
    {
        // Selon les compilateur, ajouter ./ au nom du fichier peut poser
        // des problemes de lecture du fichier
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
            // Fichier
            blobFile(absPath);
            wt->tab[i].hash = sha256file(absPath);
            wt->tab[i].mode = getChmod(absPath);
        }
        else
        {
            // Repertoire
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
    // Meme fonctionnement que saveWorkTree, cette fois on restaure les fichiers et repertoires
    // a la version correspondant au commit du WT.
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
        { // Fichier
            getChmod(absPath);
            cp(absPath, copyPath);
            // 620: rw pour utilisateur
            setMode(620, absPath);
        }
        else
        {
            if (isWorkTree(hash) == 1)
            { // Repertoire
                strcat(copyPath, ".t");
                WorkTree *nwt = ftwt(copyPath);
                restoreWorkTree(nwt, absPath);
                setMode(620, absPath);
            }
        }
    }
}

WorkTree *mergeWorkTrees(WorkTree *wt1, WorkTree *wt2, List *conflicts)
{
    WorkTree *noConflictWT = initWorkTree();
    WorkTree *tmp;

    // On cherche les elements de wt1 qui sont aussi dans wt2 et on les ajoute a la liste de conflits.
    // Le reste est ajoute au WT des elements non en conflit.
    for (int i = 0; i < wt1->n; i++)
    {
        if (inWorkTree(wt2, wt1->tab[i].name) != -1)
        {
            Cell *cell = buildCell(wt1->tab[i].name);
            insertFirst(conflicts, cell);
        }
        else
        {
            appendWorkTree(noConflictWT, wt1->tab[i].name, wt1->tab[i].hash, wt1->tab[i].mode);
        }
    }

    // On fait la meme chose pour wt2 en s'assurant de ne pas ajouter le fichier 2 fois.
    for (int j = 0; j < wt2->n; j++)
    {
        if (inWorkTree(wt1, wt2->tab[j].name) == -1)
        {
            appendWorkTree(noConflictWT, wt2->tab[j].name, wt2->tab[j].hash, wt2->tab[j].mode);
        }
    }

    return noConflictWT;
}
