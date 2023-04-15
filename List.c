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

    // On parcourt la liste jusqu'a la fin en incrementant le comteur n

    while (head != NULL)
    {
        n = n + 1;
        head = head->next;
    }

    return n;
}

void insertFirst(List *L, Cell *C)
{
    // On fait pointer la cellule suivante de C vers L puis on renvoie C, la nouvelle tete de liste
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
    // On parcourt la liste (non nulle), jusqu'a la fin et on ajoute chaque element
    // au string que l'on retourne
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

    // On parcourt la liste et des que l'on trouve l'element correspondant a l'index, on s'arrete
    while (ptr != NULL)
    {
        if (k == i)
            break;
        ptr = ptr->next;
        k = k + 1;
    }
    if (ptr == NULL)
    {
        printf("Error: Index out of range \n");
    }
    return ptr;
}

Cell *searchList(List *L, char *str)
{
    // On parcourt la liste jusqu'a trouver l'element
    List ptr = *L;
    while (ptr != NULL && strcmp(str, ptr->data) != 0)
    {
        ptr = ptr->next;
    }
    return ptr;
}

List *stol(char *s)
{
    int string_size = strlen(s);
    int increment = 0;
    int break_pos = 0;

    char *ptr;
    char *cell_content = malloc(sizeof(char) * 1000);
    List *L = initList();

    // On cherche la position de tous les | et on ajoute la chaine de characteres precedent
    // au contenu de la cellule. On s'arrete lorsqu'on trouve \n (fin de chaine) et on ajoute
    // la cellule a notre liste.
    while (increment < string_size)
    {
        ptr = strchr(s + increment, "|");
        if (ptr == NULL)
            break_pos = strlen(s) + 1;
        else
        {
            break_pos = ptr - s + 1;
        }
        memcpy(cell_content, s + increment, break_pos - increment - 1);
        cell_content[break_pos - increment - 1] = *"\0";
        increment = break_pos;
        insertFirst(L, buildCell(cell_content));
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
    List *filtered_list = initList();

    // On parcourt notre liste: si l'element correspond au pattern, on le garde, sinon
    // on le passe.
    for (Cell *ptr = *L; ptr != NULL; ptr = ptr->next)
    {
        char *c = strdup(ptr->data);
        c[strlen(pattern)] = *"\0";
        if (strcmp(c, pattern) == 0)
        {
            insertFirst(filtered_list, buildCell(ptr->data));
        }
        free(c);
    }
    return filtered_list;
}
