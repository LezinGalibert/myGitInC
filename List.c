#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int MAX_FILES = 200;
int N = 2000;

typedef struct cell
{
    char *data;
    struct cell *next;
} Cell;
typedef Cell *List;

List *initList()
{
    List *L = malloc(sizeof(List));
    *L = NULL;
    return L;
}

Cell *buildCell(char *ch)
{
    Cell *c = malloc(sizeof(Cell));
    c->data = strdup(ch);
    c->next = NULL;
    return c;
}

int listSize(List *L)
{
    List head = *L;
    int n = 0;

    while (head != NULL)
    {
        n = n + 1;
        head = head->next;
    }

    return n;
}

void insertFirst(List *L, Cell *C)
{
    C->next = *L;
    *L = C;
}

char *ctos(Cell *c)
{
    return c->data;
}

char *ltos(List *L)
{
    if (*L == NULL)
    {
        return " ";
    }
    char *ch = malloc(sizeof(char) * MAX_FILES * N);
    List ptr = *L;
    while (ptr != NULL)
    {
        strcat(ch, ptr->data);
        ptr = ptr->next;
        if (ptr != NULL)
            strcat(ch, "|");
    }
    return ch;
}

Cell *listGet(List *L, int i)
{
    Cell *ptr = *L;
    int k = 0;
    while (ptr != NULL)
    {
        if (k == i)
            break;
        ptr = ptr->next;
        k = k + 1;
    }
    if (ptr == NULL)
    {
        printf("INDEX OUT OF RANGE !!!! \n");
    }
    return ptr;
}

Cell *searchList(List *L, char *str)
{
    List ptr = *L;
    while (ptr != NULL && strcmp(str, ptr->data) != 0)
    {
        ptr = ptr->next;
    }
    return ptr;
}

List *stol(char *s)
{
    int pos = 0;
    int n_pos = 0;
    int size = strlen(s);
    int sep = "|";
    char *ptr;
    char *result = malloc(sizeof(char) * 1000);
    int end = 0;
    List *L = initList();
    while (pos < strlen(s))
    {
        ptr = strchr(s + pos, sep);
        if (ptr == NULL)
            n_pos = strlen(s) + 1;
        else
        {
            n_pos = ptr - s + 1;
        }
        memcpy(result, s + pos, n_pos - pos - 1);
        result[n_pos - pos - 1] = *"\0";
        pos = n_pos;
        insertFirst(L, buildCell(result));
    }
    return L;
}

List *ftol(char *path)
{
    char buff[N * MAX_FILES];
    FILE *f = fopen(path, "r");
    fgets(buff, N * MAX_FILES, f);
    return stol(buff);
}

void ltof(List *L, char *path)
{
    FILE *fp = fopen(path, "w");
    if (fp != NULL)
    {
        fputs(ltos(L), fp);
        fclose(fp);
    }
}

List *filterList(List *L, char *pattern)
{
    List *filtered = initList();
    for (Cell *ptr = *L; ptr != NULL; ptr = ptr->next)
    {
        char *c = strdup(ptr->data);
        c[strlen(pattern)] = *"\0";
        if (strcmp(c, pattern) == 0)
        {
            insertFirst(filtered, buildCell(ptr->data));
        }
        free(c);
    }
    return filtered;
}
