#include "Branch.c"

void myGitAdd(char *file_or_folder)
{
    WorkTree *wt;
    if (!file_exists(".add"))
    {
        createFile(".add");
        wt = initWorkTree();
    }
    else
    {
        wt = ftwt(".add");
    }
    if (file_exists(file_or_folder))
    {
        appendWorkTree(wt, file_or_folder, 0, 0);
        wttf(wt, ".add");
    }
    else
    {
        printf("file or folder %s does not exist\n", file_or_folder);
    }
}

void myGitCommit(char *branch_name, char *message)
{
    if (!file_exists(".refs"))
    {
        printf("You first need to initalise the project references");
        return;
    }
    if (!file_exists_in_dir(".refs", branch_name))
    {
        printf("Branch %s does not exist.", branch_name);
        return;
    }
    char *last_hash = getRef(branch_name);
    char *head_hash = getRef("HEAD");
    if (strcmp(last_hash, head_hash) != 0)
    {
        printf("HEAD must point to the last commit of the current branch");
        return;
    }
    WorkTree *wt = ftwt(".add");
    char *hashwt = saveWorkTree(wt, ".");
    Commit *c = createCommit(hashwt);
    if (strlen(last_hash) != 0)
    {
        commitSet(c, "predecessor", last_hash);
    }
    if (message != NULL)
    {
        commitSet(c, "message", message);
    }
    char *hashc = blobCommit(c);
    createUpdateRef(branch_name, hashc);
    createUpdateRef("HEAD", hashc);
    system("rm .add");
}

void myGitCheckoutBranch(char *branch)
{
    // Change current_branch :
    FILE *f = fopen(".current_branch", "w");
    fprintf(f, "%s", branch);
    fclose(f);

    char *hash_commit = getRef(branch);
    createUpdateRef("HEAD", hash_commit);
    restoreCommit(hash_commit);
}

void myGitCheckoutCommit(char *pattern)
{
    List *L = getAllCommits();
    List *filtred_list = filterList(L, pattern);
    if (listSize(filtred_list) == 1)
    {
        char *commit_hash = (listGet(filtred_list, 0))->data;
        createUpdateRef("HEAD", commit_hash);
        restoreCommit(commit_hash);
    }
    else
    {
        if (listSize(filtred_list) == 0)
        {
            printf("No pattern matching. \n");
        }
        else
        {
            printf("Multiple match found: \n");
            for (Cell *ptr = *filtred_list; ptr != NULL; ptr = ptr->next)
            {
                printf("−>%s\n", ptr->data);
            }
        }
    }
}

void *createDeletionCommit(char *branch, List *conflicts, char *message)
{
    myGitCheckoutBranch(branch);
    char *curr_branch = getCurrentBranch();
    char *curr_commit_hash = getRef(curr_branch);
    char *curr_wt_hash = hashToPath(commitGet(ftc(hashToPathCommit(curr_commit_hash)), "tree"));
    strcat(curr_wt_hash, ".t");
    WorkTree *curr_wt = ftwt(curr_wt_hash);
    system("rm .add");

    for (int i = 0; i < curr_wt->n; i++)
    {
        if (searchList(conflicts, curr_wt->tab[i].name) == NULL)
        {
            myGitAdd(curr_wt->tab[i].name);
        }
    }
    myGitCommit(branch, message);
    myGitCheckoutBranch(curr_branch);
}

int main(int argc, char *argv[])
{
    if (strcmp(argv[1], "init") == 0)
    {
        initRefs();
        initBranch();
    }
    if (strcmp(argv[1], "refs-list") == 0)
    {
        printf("REFS: \n");
        if (file_exists(".refs"))
        {
            List *L = listdir(".refs");
            for (Cell *ptr = *L; ptr != NULL; ptr = ptr->next)
            {
                if (ptr->data[0] == *".")
                    continue;
                char *content = getRef(ptr->data);
                printf("−%s\t%s\n", ptr->data, content);
            }
        }
    }
    if (strcmp(argv[1], "create-ref") == 0)
    {
        createUpdateRef(argv[2], argv[3]);
    }
    if (strcmp(argv[1], "delete-ref") == 0)
    {
        deleteRef(argv[2]);
    }
    if (strcmp(argv[1], "add") == 0)
    {
        for (int i = 2; i < argc; i++)
        {
            myGitAdd(argv[i]);
        }
    }
    if (strcmp(argv[1], "clear-add") == 0)
    {
        system("rm .add");
    }
    if (strcmp(argv[1], "add-list") == 0)
    {
        printf("Staging area: \n");
        if (file_exists(".add"))
        {
            WorkTree *wt = ftwt(".add");
            printf("%s\n", wtts(wt));
        }
    }
    if (strcmp(argv[1], "commit") == 0)
    {
        if (strcmp(argv[3], "−m") == 0)
        {
            myGitCommit(argv[2], argv[4]);
        }
        else
        {
            myGitCommit(argv[2], NULL);
        }
    }
    if (strcmp(argv[1], "get-current-branch") == 0)
    {
        printf("%s", getCurrentBranch());
    }

    if (strcmp(argv[1], "branch") == 0)
    {
        if (!branchExists(argv[2]))
            createBranch(argv[2]);
        else
            printf("The branch already exists.");
    }

    if (strcmp(argv[1], "branch-print") == 0)
    {
        if (!branchExists(argv[2]))
            printf("The branch does not exist.");
        else
            printBranch(argv[2]);
    }

    if (strcmp(argv[1], "checkout-branch") == 0)
    {
        if (!branchExists(argv[2]))
            printf("The branch does not exist.");
        else
            myGitCheckoutBranch(argv[2]);
    }
    if (strcmp(argv[1], "checkout−commit") == 0)
    {
        myGitCheckoutCommit(argv[2]);
    }
    if (strcmp(argv[1], "merge") == 0)
    {
        if (!branchExists(argv[2]))
            printf("The branch does not exist.");
        else
        {
            List conflicts = *merge(argv[2], "Branches have been successfully merged.");

            if (conflicts != NULL)
            {
                char *choice = "remote";
                printf("Several the branches are in conflicts, please choose the merge method:\n");
                printf("- remote: commit conflicts on branch %s\n", argv[2]);
                printf("- current: commit conflicts on branch %s\n", getCurrentBranch());
                printf("- custom: choose where to commit conflicts\n");
                // scanf("%s", choice);

                if (strcmp(choice, "remote") == 0)
                {
                    createDeletionCommit(argv[2], &conflicts, "Deletion commit on remote branch.");
                    merge(argv[2], "Branches have been successfully merged.");
                }
                else if (strcmp(choice, "current") == 0)
                {
                    createDeletionCommit(getCurrentBranch(), &conflicts, "Deletion commit on current branch.");
                    merge(argv[2], "Branches have been successfully merged.");
                }
                else if (strcmp(choice, "custom") == 0)
                {
                    List *mergeOnRemote = initList();
                    List *mergeOnCurrent = initList();

                    while (conflicts != NULL)
                    {
                        char conflictChoice[20];
                        printf("This file is in conflict: %s.\n", conflicts->data);
                        printf("On which branch would you like to commit the conflict?\n");
                        printf("- remote\n");
                        printf("- current");
                        scanf("%s", conflictChoice);

                        Cell *cell = buildCell(conflicts->data);

                        if (strcmp(conflictChoice, "remote"))
                        {
                            insertFirst(mergeOnRemote, cell);
                            conflicts = conflicts->next;
                        }
                        else if (strcmp(conflictChoice, "current"))
                        {
                            insertFirst(mergeOnCurrent, cell);
                            conflicts = conflicts->next;
                        }
                        else
                        {
                            printf("Choice not recognised.");
                        }
                    }
                    merge(argv[2], "Branches have been successfully merged.");
                }
                else
                {
                }
            }
        }
    }
    return 0;
}
