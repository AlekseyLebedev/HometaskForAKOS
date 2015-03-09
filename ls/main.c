#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <strings.h>

#define VECTOR_TYPE char

#include "vector.h"

char *const ERROR_PREFIX = "ls: Error in file \"";
char *const ERROR_INFIX = "\"";
static char *const ERROR_STRING = "<error>";

static const int MEMORY_ALLOC_ERROR = 3;
static const int TIME_END_TRUNCATE = 9;

int checkNull(void *pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Wrong memory alloc.\n");
        return 1;
    } else
        return 0;
}

char *strconcat(int count, ...) {
    va_list lst;
    va_start(lst, count);
    size_t length = 0;
    for (int i = 0; i < count; ++i) {
        length += strlen(va_arg(lst, char*));
    }
    va_end(lst);
    va_start(lst, count);
    char *result = malloc((length + 1) * sizeof(char));
    if (result) {
        result[0] = '\0';
        for (int i = 0; i < count; ++i) {
            strcat(result, va_arg(lst, char*));
        }
    }
    return result;
}

void printError(const char *name) {
    char *message = strconcat(3, ERROR_PREFIX, name, ERROR_INFIX);
    if (!checkNull(message)) {
        perror(message);
        free(message);
    }
}


void printErrorInfo(const char *name, const char *info) {
    char *message = strconcat(4, ERROR_PREFIX, name, ERROR_INFIX, info);
    if (!checkNull(message)) {
        fprintf(stderr, "%s", message);
    }
}

struct DirInfo {
    size_t blocks;
    struct vector *output;
    size_t groupLength;
    size_t userLength;
    size_t linksLength;
    size_t sizeLength;
    size_t timeLength;
};

void clearDirInfo(struct DirInfo *dirInfo) {
    vectorClear(dirInfo->output);
    dirInfo->blocks = 0;
    dirInfo->timeLength = 0;
    dirInfo->groupLength = 0;
    dirInfo->linksLength = 0;
    dirInfo->sizeLength = 0;
    dirInfo->userLength = 0;
}

void printFilesList(struct DirInfo *dirInfo) {
    for (int j = 0; j < dirInfo->output->size; ++j) {
        char *line = dirInfo->output->array[j];
        printf("%s ", line);
        line += 11;
        printf("%*s ", (int) dirInfo->linksLength, line);
        line += strlen(line) + 1;
        printf("%*s ", (int) dirInfo->groupLength, line);
        line += strlen(line) + 1;
        printf("%*s ", (int) dirInfo->userLength, line);
        line += strlen(line) + 1;
        printf("%*s ", (int) dirInfo->sizeLength, line);
        line += strlen(line) + 1;
        printf("%*s ", (int) dirInfo->timeLength, line);
        line += strlen(line) + 1;
        printf("%s\n", line);
    }
}

void setMax(size_t value, size_t *lnk) {
    if (*lnk < value)
        *lnk = value;
}

void printAccess(struct stat info, char *line, char val, int flag, int index) {
    line[index] = (info.st_mode & flag) ? val : '-';
}

int printInfo(char *filename, struct DirInfo *dirInfo) {
    if (!strlen(filename))
        return 1;
    struct stat information;
    if (lstat(filename, &information)) {
        printError(filename);
        return 0;
    } else {
        int isdir = S_ISDIR(information.st_mode);
        dirInfo->blocks += information.st_blocks >> 1;
        char *group;
        struct group *groupinfo = getgrgid(information.st_gid);
        if (groupinfo) {
            group = groupinfo->gr_name;
        } else {
            printError(filename);
            group = ERROR_STRING;
        }
        char *user;
        struct passwd *userinfo = getpwuid(information.st_uid);
        if (groupinfo) {
            user = userinfo->pw_name;
        } else {
            printError(filename);
            user = ERROR_STRING;
        }
        char links[20];
        sprintf(links, "%ld", information.st_nlink);
        char size[20];
        sprintf(size, "%ld", information.st_size);
        char *time = ctime(&information.st_mtime);
        if (!time) {
            printErrorInfo(filename, "Wrong time results");
            return isdir;
        }
        char *oldtime = time + 4;
        size_t oldtimelength = strlen(oldtime);
        time = malloc(oldtimelength + 1 - TIME_END_TRUNCATE);
        if (checkNull(time))
            return isdir;
        strncpy(time, oldtime, oldtimelength - TIME_END_TRUNCATE);
        time[oldtimelength - TIME_END_TRUNCATE] = '\0';
        int wasColored = 0;
        char *searchName = filename + strlen(filename);
        while (*searchName != '/')
            searchName--;
        searchName++;
        char *name = malloc(strlen(searchName) + 20);
        if (checkNull(name)) {
            free(time);
            return isdir;
        }
        name[0] = '\0';
        if (S_IXUSR & information.st_mode) {
            strcpy(name, "\x1b[1;32m");
            wasColored = 1;
        }
        if (S_ISDIR(information.st_mode)) {
            strcpy(name, "\x1b[1;34m");
            wasColored = 1;
        }
        int islnk = S_ISLNK(information.st_mode);
        if (islnk) {
            strcpy(name, "\x1b[1;36m");
            wasColored = 1;
        }
        strcat(name, searchName);
        if (wasColored)
            strcat(name, "\x1b[0m");
        if (islnk) {
            char *lnkName = name;
            size_t lnkcapacity = 1024;
            ssize_t lnklength;
            char *destination = NULL;
            do {
                if (destination)
                    free(destination);
                destination = malloc(lnkcapacity * sizeof(char));
                if (checkNull(destination)) {
                    lnklength = -1;
                    break;
                }
                lnklength = readlink(filename, destination, lnkcapacity);
            } while (lnklength == lnkcapacity);
            if (lnklength >= 0) {
                destination[lnklength] = '\0';
                if (lnklength == 1)
                    lnklength += 2;
                char *search = destination + lnklength - 2;
                while (search > destination && *search != '/')
                    search--;
                name = strconcat(3, name, " -> ", search);
            } else {
                name = NULL;
                printError(filename);
            }
            if (destination)
                free(destination);
            free(lnkName);
            if (checkNull(name)) {
                return isdir;
            }
        }
        size_t timeLength = strlen(time);
        size_t userLength = strlen(user);
        size_t nameLength = strlen(name);
        size_t groupLength = strlen(group);
        size_t sizeLength = strlen(size);
        size_t linksLength = strlen(links);
        setMax(userLength, &dirInfo->userLength);
        setMax(sizeLength, &dirInfo->sizeLength);
        setMax(timeLength, &dirInfo->timeLength);
        setMax(groupLength, &dirInfo->groupLength);
        setMax(linksLength, &dirInfo->linksLength);
        char *output = malloc((17 + timeLength + userLength + nameLength + groupLength
                + sizeLength + linksLength) * sizeof(char));
        if (checkNull(output))
            return isdir;
        output[0] = isdir ? 'd' : islnk ? 'l' : '-';
        printAccess(information, output, 'r', S_IRUSR, 1);
        printAccess(information, output, 'w', S_IWUSR, 2);
        printAccess(information, output, 'x', S_IXUSR, 3);
        printAccess(information, output, 'r', S_IRGRP, 4);
        printAccess(information, output, 'w', S_IWGRP, 5);
        printAccess(information, output, 'x', S_IXGRP, 6);
        printAccess(information, output, 'r', S_IROTH, 7);
        printAccess(information, output, 'w', S_IWOTH, 8);
        printAccess(information, output, 'x', S_IXOTH, 9);
        int index = 10;
        output[index] = '\0';
        ++index;
        strcpy(output + index, links);
        index += linksLength;
        output[index] = '\0';
        ++index;
        strcpy(output + index, group);
        index += groupLength;
        output[index] = '\0';
        ++index;
        strcpy(output + index, user);
        index += userLength;
        output[index] = '\0';
        ++index;
        strcpy(output + index, size);
        index += sizeLength;
        output[index] = '\0';
        ++index;
        strcpy(output + index, time);
        index += timeLength;
        output[index] = '\0';
        ++index;
        strcpy(output + index, name);
        index += nameLength;
        output[index] = '\0';
        free(time);
        free(name);
        vectorAdd(dirInfo->output, output);
        return isdir;
    }
}


int cmpstring(const void *a, const void *b) {
    return strcmp(*(char *const *) a, *(char *const *) b);
}

void skipinfo(char **pointer) {
    for (int i = 0; i < 6; ++i) {
        while (**pointer) {
            (*pointer)++;
        }
        (*pointer)++;
    }
    if (**pointer == '\033') {
        while (**pointer != 'm')
            ++(*pointer);
        ++(*pointer);
    }
}

int cmpfiles(const void *a, const void *b) {
    char *first = *(char **) a;
    char *second = *(char **) b;
    skipinfo(&first);
    skipinfo(&second);
    return strcasecmp((const char *) first, (const char *) second);
}

void printdir(const char *name, struct DirInfo *dirInfo) {
    DIR *directory = opendir(strlen(name) ? name : "/");
    if (directory) {
        struct vector *directories = vectorCreate();
        if (checkNull(directories)) {
            return;
        }
        printf("%s:\n", name);
        clearDirInfo(dirInfo);
        struct dirent *entry;
        while (entry = readdir(directory)) {
            if (entry->d_name[0] != '.') {
                char *filename = strconcat(3, name, "/", entry->d_name);
                if (printInfo(filename, dirInfo)) {
                    if (vectorAdd(directories, filename)) {
                        printErrorInfo(filename, "Memory error");
                        free(filename);
                        directories = NULL;
                        break;
                    }
                } else {
                    free(filename);
                }
            }
        }
        if (errno != 0) {
            printError(name);
        }
        printf("total: %zu\n", dirInfo->blocks);
        qsort(dirInfo->output->array, dirInfo->output->size, sizeof(VECTOR_TYPE *), cmpfiles);
        printFilesList(dirInfo);
        printf("\n");
        if (closedir(directory)) {
            printError(name);
        }
        if (directories) {
            qsort(directories->array, directories->size, sizeof(VECTOR_TYPE *), cmpstring);
            for (int i = 0; i < directories->size; ++i) {
                printdir(directories->array[i], dirInfo);
            }
            vectorFree(directories);
        }
    } else {
        printError(name);
    }
}

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Usage: ls [path]\n");
        return 2;
    }
    struct DirInfo dirInfo;
    dirInfo.output = vectorCreate();
    if (checkNull(dirInfo.output)) {
        vectorFree(dirInfo.output);
        return MEMORY_ALLOC_ERROR;
    }
    if (argc > 1) {
        char *name = malloc(strlen(argv[1]) + 3);
        if (checkNull(name)) {
            vectorFree(dirInfo.output);
            return MEMORY_ALLOC_ERROR;
        }
        if (argv[1][0] == '/') {
            strcpy(name, argv[1]);
        } else {
            strcpy(name, "./");
            strcat(name, argv[1]);
        }
        size_t lastChar = strlen(name) - 1;
        if (name[lastChar] == '/')
            name[lastChar] = '\0';
        clearDirInfo(&dirInfo);
        if (printInfo(name, &dirInfo)) {
            printdir(name, &dirInfo);
        }
        else {
            if (dirInfo.output->size)
                printFilesList(&dirInfo);
        }
        free(name);
    } else {
        printdir(".", &dirInfo);
    }
    vectorFree(dirInfo.output);
    return 0;
}