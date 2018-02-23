#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define PATHSIZE 512

int listdir(const char *path) {
    struct dirent *entry;
    DIR *dp;

    printf("parsing directory: %s\n", path);

    dp = opendir(path);
    if (dp == NULL) {
        perror("opendir: Path does not exist or could not be read.");
        return -1;
    }

    while ((entry = readdir(dp))) {

        if (entry->d_type == DT_DIR) {
                if ((!(strcmp(entry->d_name, "."))) ||
                     (!(strcmp(entry->d_name, ".."))) ||
                     (!(strcmp(entry->d_name, "redhat"))) ||
                     (!(strcmp(entry->d_name, "configs"))) ||
                     (!(strcmp(entry->d_name, "Documentation")))) {
                        continue;
                }
                char *newpath = (char *)malloc(PATHSIZE * sizeof(char));
                strcpy(newpath, path);
                strcat(newpath, "/");
                strcat(newpath, entry->d_name);
                listdir(newpath);
                free(newpath);
        } else {
                char *name = entry->d_name;
                char *cp = &name[strlen(name) - 2];
                if (strcmp(cp, ".i"))
                        continue;
        }

        printf("%s\n", entry->d_name);
    }

    closedir(dp);
    return 0;
}

int main (int argc, char *argv[])
{
        return listdir(argv[1]);
}
