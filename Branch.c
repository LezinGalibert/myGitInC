#include "Commit.c"

void initBranch()
{
    FILE *f = fopen(".current_branch", "w");
    fputs("master", f);
    fclose(f);
}

int branchExists(char *branch)
{
    List *refs = listdir(".refs");
    return searchList(refs, branch) != NULL;
}

void createBranch(char *branch)
{
    char *hash = getRef("HEAD");
    createUpdateRef(branch, hash);
}

char *getCurrentBranch()
{
    FILE *f = fopen(".current_branch", "r");
    char *buff = malloc(sizeof(char) * 100);
    fscanf(f, "%s", buff);
    fclose(f);

    return buff;
}

void printBranch(char *branch)
{
    char *commit_hash = getRef(branch);
    Commit *c = ftc(hashToPathCommit(commit_hash));
    while (c != NULL)
    {
        if (commitGet(c, "message") != NULL)
            printf("%s−>%s\n", commit_hash, commitGet(c, "message"));
        else
            printf("%s\n", commit_hash);
        if (commitGet(c, "predecessor") != NULL)
        {
            commit_hash = commitGet(c, "predecessor");
            c = ftc(hashToPathCommit(commit_hash));
        }
        else
        {
            c = NULL;
        }
    }
}

List *branchList(char *branch)
{
    List *L = initList();
    char *commit_hash = getRef(branch);
    Commit *c = ftc(hashToPathCommit(commit_hash));
    while (c != NULL)
    {
        insertFirst(L, buildCell(commit_hash));
        if (commitGet(c, "predecessor") != NULL)
        {
            commit_hash = commitGet(c, "predecessor");
            c = ftc(hashToPathCommit(commit_hash));
        }
        else
        {
            c = NULL;
        }
    }
    return L;
}

List *getAllCommits()
{
    List *L = initList();
    List *content = listdir(".refs");
    for (Cell *ptr = *content; ptr != NULL; ptr = ptr->next)
    {
        if (ptr->data[0] == *".")
            continue;
        List *list = branchList(ptr->data);
        Cell *cell = *list;
        while (cell != NULL)
        {
            if (searchList(L, cell->data) == NULL)
            {
                insertFirst(L, buildCell(cell->data));
            }
            cell = cell->next;
        }
    }
    return L;
}

List *merge(char *remote_branch, char *message)
{
    List *conflicts = initList();
    char *curr_commit_hash = getRef(getCurrentBranch());
    char *remote_commit_hash = getRef(remote_branch);

    char *curr_commit_path = hashToPathCommit(curr_commit_hash);
    char *remote_commit_path = hashToPathCommit(remote_commit_hash);

    char *curr_wt_hash = hashToPath(commitGet(ftc(curr_commit_path), "tree"));
    char *remote_wt_hash = hashToPath(commitGet(ftc(remote_commit_path), "tree"));

    strcat(curr_wt_hash, ".t");
    strcat(remote_wt_hash, ".t");

    WorkTree *curr_wt = ftwt(curr_wt_hash);
    WorkTree *remote_wt = ftwt(remote_wt_hash);

    WorkTree *conflictFreeWT = mergeWorkTrees(curr_wt, remote_wt, conflicts);

    if (listSize(conflicts) != 0)
    {
        return conflicts;
    }
    else
    {
        char *hashwt = saveWorkTree(conflictFreeWT, ".");
        Commit *merge_commit = createCommit(hashwt);
        commitSet(merge_commit, "predecessor", curr_commit_hash);
        commitSet(merge_commit, "merged_predecessor", remote_commit_hash);
        commitSet(merge_commit, "message", message);

        char *hashc = blobCommit(merge_commit);
        createUpdateRef(getCurrentBranch(), hashc);
        createUpdateRef("HEAD", hashc);
        deleteRef(remote_branch);

        restoreWorkTree(conflictFreeWT, ".");

        return NULL;
    }
}