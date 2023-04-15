#include "Branch.c"

void myGitAdd(char *file_or_folder)
{
    WorkTree *wt;
    // On cree le repertoire .add s'il n'existe pas
    if (!file_exists(".add"))
    {
        createFile(".add");
        wt = initWorkTree();
    }
    else
    {
        // Sinon on construit un WT base sur les fichiers dans la zone de preparation
        wt = ftwt(".add");
    }

    // On s'assure que le fichier / dossier existe bien et on l'ajoute a notre WT
    if (file_exists(file_or_folder))
    {
        appendWorkTree(wt, file_or_folder, 0, 0);
        wttf(wt, ".add");
    }
    else
    {
        printf("Error: File or folder %s does not exist\n", file_or_folder);
    }
}

void myGitCommit(char *branch_name, char *message)
{

    // On s'assure que les ref ont bien ete initialisees et que la branche existe
    if (!file_exists(".refs"))
    {
        printf("Error: Initialize the project references first.");
        return;
    }
    if (!file_exists_in_dir(".refs", branch_name))
    {
        printf("Error: branch %s does not exist.", branch_name);
        return;
    }

    // On s'assure que HEAD pointe bien vers le meme commit que branch_name
    char *remote_hash = getRef(branch_name);
    char *head_hash = getRef("HEAD");
    if (strcmp(remote_hash, head_hash) != 0)
    {
        printf("HEAD must point to the last commit of the current branch");
        return;
    }

    // Si tout est bon, on cree notre WT a partir des fichiers dans la zone de preparation (.add)
    // et on cree notre commit dont le precedesseur sera le commit vers lequel HEAD pointait
    WorkTree *wt = ftwt(".add");
    char *wt_hash = saveWorkTree(wt, ".");
    Commit *c = createCommit(wt_hash);
    if (strlen(remote_hash) != 0)
    {
        commitSet(c, "predecessor", remote_hash);
    }
    if (message != NULL)
    {
        commitSet(c, "message", message);
    }

    // On met les refs a jour et on vide la zone de preparation
    char *commit_hash = blobCommit(c);
    createUpdateRef(branch_name, commit_hash);
    createUpdateRef("HEAD", commit_hash);
    system("rm .add");
}

void myGitCheckoutBranch(char *branch)
{
    // Change la branche courante en "branch"
    FILE *f = fopen(".current_branch", "w");
    fprintf(f, "%s", branch);
    fclose(f);

    // On recupere le hash du dernier commit sur "branch" puis on restore les fichiers
    // correspondant a ce commit
    char *commit_hash = getRef(branch);
    createUpdateRef("HEAD", commit_hash);
    restoreCommit(commit_hash);
}

void myGitCheckoutCommit(char *pattern)
{
    // Recupere la liste de tous les commits et cherche lequel correspond a "pattern"
    List *commits = getAllCommits();
    List *filtered_list = filterList(commits, pattern);

    // Si on trouve 1 match, on change le commit vers lequel HEAD pointe en ce commit
    // Sinon on retourne tous les match sans rien faire
    if (listSize(filtered_list) == 1)
    {
        char *commit_hash = (listGet(filtered_list, 0))->data;
        createUpdateRef("HEAD", commit_hash);
        restoreCommit(commit_hash);
    }
    else
    {
        if (listSize(filtered_list) == 0)
        {
            printf("No commit matching the provided pattern.\n");
        }
        else
        {
            printf("Multiple match found for pattern %s:\n", pattern);
            for (Cell *ptr = *filtered_list; ptr != NULL; ptr = ptr->next)
            {
                printf("−>%s\n", ptr->data);
            }
        }
    }
}

void *createDeletionCommit(char *branch, List *conflicts, char *message)
{
    // On sauve la branch courante, au cas ou elle differe de master pour pouvoir se repositionner
    // dessus a la fin avant de se positionner sur la branche a fusionner
    char *curr_branch = getCurrentBranch();
    myGitCheckoutBranch(branch);

    // On recupere le WT de la branche courante
    char *curr_commit_hash = getRef(branch);
    char *curr_wt_hash = hashToPath(commitGet(ftc(hashToPathCommit(curr_commit_hash)), "tree"));
    strcat(curr_wt_hash, ".t");
    WorkTree *curr_wt = ftwt(curr_wt_hash);
    system("rm .add");

    // On ajoute tous les fichiers du WT qui ne sont pas en conflit dans la zone de preparation
    for (int i = 0; i < curr_wt->n; i++)
    {
        if (searchList(conflicts, curr_wt->tab[i].name) == NULL)
        {
            myGitAdd(curr_wt->tab[i].name);
        }
    }

    // On cree le commit de deletion sur notre remote
    // et on se repositionne sur la branche courante initiale
    myGitCommit(branch, message);
    myGitCheckoutBranch(curr_branch);
}

int main(int argc, char *argv[])
{
    if (strcmp(argv[1], "init") == 0)
    {
        // Intialise les refs et branches
        // ./myGit init
        initRefs();
        initBranch();
        system('mkdir tmp');
    }
    if (strcmp(argv[1], "refs-list") == 0)
    {
        // Affiche les references stockees dans .refs
        // ./myGit refs-list
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
        // Met a jour une references avec un nouveau hash de commit
        // ./myGit create-ref new_ref commit_hash
        createUpdateRef(argv[2], argv[3]);
    }
    if (strcmp(argv[1], "delete-ref") == 0)
    {
        // Efface une reference
        // ./myGit delete-refs ref_to_delete
        deleteRef(argv[2]);
    }
    if (strcmp(argv[1], "add") == 0)
    {
        // Ajoute les fichiers passes en commentaire dans la zone de preparation
        // ./myGit add fichier1 fichier2 ...
        for (int i = 2; i < argc; i++)
        {
            myGitAdd(argv[i]);
        }
    }
    if (strcmp(argv[1], "clear-add") == 0)
    {
        // Vide la zone de preparation
        // ./myGit clear-add
        system("rm .add");
    }
    if (strcmp(argv[1], "add-list") == 0)
    {
        // Affiche le contenu de la zone de preparation
        // ./myGit add-list
        printf("Staging area: \n");
        if (file_exists(".add"))
        {
            WorkTree *wt = ftwt(".add");
            printf("%s\n", wtts(wt));
        }
    }
    if (strcmp(argv[1], "commit") == 0)
    {
        // Commit le contenu de la zone de preparation sur la branche specifiee.
        // On doit etre sur cette branche pour pouvoir proceder.
        if (argc >= 4)
        {
            // ./myGit commit branch -m commit_message
            if (strcmp(argv[3], "-m") == 0)
            {
                myGitCommit(argv[2], argv[4]);
            }
            else
            {
                printf("Error: Please provide one commit message.");
            }
        }
        else
        {
            // ./myGit commit master
            myGitCommit(argv[2], NULL);
        }
    }
    if (strcmp(argv[1], "get-current-branch") == 0)
    {
        // Affiche le nom de la branche courante
        // ./myGit get-current-branch
        printf("%s", getCurrentBranch());
    }

    if (strcmp(argv[1], "branch") == 0)
    {
        // Cree une nouvelle branche si elle n'existe pas
        // ./myGit branch new_branch
        if (!branchExists(argv[2]))
            createBranch(argv[2]);
        else
            printf("Error: The branch already exists.");
    }

    if (strcmp(argv[1], "branch-print") == 0)
    {
        // Affiche le contenu de la branche
        // ./myGit branche-print
        if (!branchExists(argv[2]))
            printf("The branch does not exist.");
        else
            printBranch(argv[2]);
    }

    if (strcmp(argv[1], "checkout-branch") == 0)
    {
        // Change la branche courante si elle existe
        // ./myGit checkout-branch new_current_branch
        if (!branchExists(argv[2]))
            printf("The branch does not exist.");
        else
            myGitCheckoutBranch(argv[2]);
    }
    if (strcmp(argv[1], "checkout−commit") == 0)
    {
        // Restore les fichier dans l'etat du commit specifie
        // ./myGit checkout-commit old_commit_hash
        myGitCheckoutCommit(argv[2]);
    }
    if (strcmp(argv[1], "merge") == 0)
    {
        // Fusionne la branche specifiee avec la branche courante
        // ./myGit merge branch_to_merge
        if (!branchExists(argv[2]))
            printf("Error: The branch does not exist.");
        else
        {
            char *currentBranch = getCurrentBranch();

            if (strcmp(currentBranch, argv[2]) == 0)
            {
                printf("Error: Remote branch to merge must be different than current branch.");
            }
            else
            {
                // On identifie les fichiers en conflit
                List conflicts = *merge(argv[2], "Branches have been successfully merged.");

                // Si pas de conflit, on a termine
                if (conflicts != NULL)
                {

                    // Sinon on offre 3 choix a l'utilisateur: mettre le commit de deletion sur
                    // la branche remote, courante ou choisir au cas pas cas
                    int size = listSize(&conflicts);
                    char *choice;
                    printf("%d files are in conflicts, please choose the merge method:\n", size);
                    printf("- remote: commit conflicts on branch %s\n", argv[2]);
                    printf("- current: commit conflicts on branch %s\n", getCurrentBranch());
                    printf("- custom: choose where to commit conflicts\n");
                    printf("Your choice:\t");
                    scanf("%s", choice);

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
                        // Si l'utilisateur veut decider au cas par cas, on le laisse choisir pour chaque
                        // conflit
                        List *mergeOnRemote = initList();
                        List *mergeOnCurrent = initList();

                        while (conflicts != NULL)
                        {
                            char conflictChoice[20];
                            printf("This file is in conflict: %s.\n", conflicts->data);
                            printf("On which branch would you like to commit the conflict?\n");
                            printf("- remote\n");
                            printf("- current\n");
                            printf("Your choice:\t");
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
                                printf("Error: Choice not recognised, please choose between: remote / current.");
                            }
                        }

                        // On verifie que chaque liste contient au moins un fichier et on cree le commit
                        // de deletion sur chaque branche

                        if (mergeOnRemote != NULL)
                        {
                            createDeletionCommit(argv[2], &mergeOnRemote, "Deletion commit on remote branch.");
                        }

                        if (mergeOnCurrent != NULL)
                        {
                            createDeletionCommit(getCurrentBranch(), &mergeOnCurrent, "Deletion commit on current branch.");
                        }
                        merge(argv[2], "Branches have been successfully merged.");
                    }
                    else
                    {
                        printf("Error: Choice not recognised, please choose between: remote / current / custom.");
                    }
                }
            }
        }
    }
    return 0;
}
