#include "WorkTree.c"

int Ncommit = 100;
int size = 1000;
int MESSAGE_SIZE = 2000;

typedef struct key_value_pair
{
    char *key;
    char *value;
} kvp;

typedef struct hash_table
{
    kvp **T;
    int n;
    int size;
} HashTable;
typedef HashTable Commit;

kvp *createKeyVal(char *key, char *val)
{
    kvp *k = malloc(sizeof(kvp));
    k->key = strdup(key);
    k->value = strdup(val);
    return k;
}
void freeKeyVal(kvp *kv)
{
    free(kv->key);
    free(kv->value);
    free(kv);
}

char *kvts(kvp *k)
{
    char *buff = malloc(sizeof(char) * 100);
    sprintf(buff, "%s : %s", k->key, k->value);
    return buff;
}
kvp *stkv(char *str)
{
    char key[100], val[100];
    sscanf(str, "%s : %s", key, val);
    return createKeyVal(key, val);
}

Commit *initCommit(int ncommit)
{
    Commit *c = malloc(sizeof(Commit));
    c->T = malloc(size * sizeof(kvp *));
    c->size = ncommit;
    for (int i = 0; i < c->size; i++)
    {
        c->T[i] = NULL;
    }
    c->n = 0;
    return c;
}

void commitSet(Commit *c, char *key, char *value)
{
    // On cherche la position potentielle de key grace a notre fonction de hashing.
    int p = hashF(key) % c->size;
    while (c->T[p] != NULL)
    {
        // Si un element occupe deja cette position, on procede par probing lineaire
        // en incrementant l'indice de 1 jusqu'a trouver une place libre
        p = (p + 1) % c->size;
    }
    c->T[p] = createKeyVal(key, value);
    c->n++;
}

Commit *createCommit(char *hash)
{
    // Genere un commit qui pointera vers le WT represente par hash
    Commit *c = initCommit(Ncommit);
    commitSet(c, "tree", hash);
    return c;
}

char *commitGet(Commit *c, char *key)
{
    // Recupere la valeur stockee dans le commit correspondant a key
    int p = hashF(key) % c->size;

    // On doit s'arreter lorsque l'on a parcouru tout le tableau
    int attempt = 0;
    while (c->T[p] != NULL && attempt < c->size)
    {
        if (strcmp(c->T[p]->key, key) == 0)
        {
            char *val = c->T[p]->value;
            return c->T[p]->value;
        }
        p = (p + 1) % c->size; // probing lineaire
        attempt = attempt + 1;
    }
    return NULL;
}

char *cts(Commit *c)
{
    char *str = malloc(sizeof(char) * 100 * c->n);
    for (int i = 0; i < c->size; i++)
    {
        if (c->T[i] != NULL)
        {
            strcat(str, kvts(c->T[i]));
            strcat(str, "\n");
        }
    }
    return str;
}

Commit *stc(char *ch)
{
    int pos = 0;
    int n_pos = 0;
    int sep = *"\n";
    char *ptr;
    char *result = malloc(sizeof(char) * 10000);
    Commit *c = initCommit(10);
    while (pos < strlen(ch))
    {
        ptr = strchr(ch + pos, sep);
        if (ptr == NULL)
            n_pos = strlen(ch) + 1;
        else
            n_pos = ptr - ch + 1;
        memcpy(result, ch + pos, n_pos - pos - 1);
        result[n_pos - pos - 1] = *"\0";
        pos = n_pos;
        kvp *elem = stkv(result);
        commitSet(c, elem->key, elem->value);
    }
    return c;
}

Commit *ftc(char *file)
{
    char buff[MESSAGE_SIZE];
    char *all = malloc(sizeof(char) * MESSAGE_SIZE);
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
        printf("ERROR: File does not exist\n");
        return NULL;
    }
    while (fgets(buff, N, f) != NULL)
    {
        strcat(all, buff);
    }
    Commit *c = stc(all);
    return c;
}
void ctf(Commit *c, char *file)
{
    FILE *fp = fopen(file, "w");
    if (fp != NULL)
    {
        fputs(cts(c), fp);
        fclose(fp);
    }
}

char *blobCommit(Commit *c)
{
    // On cree un fichier temporaire dans /tmp, on hash le contenu du commit puis
    // le store dans le fichier temporaire apres l'avoir renome en suivant la convention
    char tmp[100] = "tmp/myfileXXXXXX";
    int _ = mkstemp(tmp);
    ctf(c, tmp);

    char *hash = sha256file(tmp);
    char *ch = hashToFile(hash);
    strcat(ch, ".c");
    cp(ch, tmp);

    return hash;
}

char *hashToPathCommit(char *hash)
{
    // Scinde le hash en dossier / fichier et rajoute .c pour recuperer
    // le fichier correspondant au commit hashe
    char *buff = malloc(sizeof(char) * 100);
    sprintf(buff, "%s.c", hashToPath(hash));
    return buff;
}

void restoreCommit(char *hash_commit)
{
    // Recupere le WT correspondant au commit hashe et le restaure dans le dossier courant
    char *commit_path = hashToPathCommit(hash_commit);
    Commit *c = ftc(commit_path);
    char *tree_hash = strcat(hashToPath(commitGet(c, "tree")), ".t");
    WorkTree *wt = ftwt(tree_hash);
    restoreWorkTree(wt, ".");
}
