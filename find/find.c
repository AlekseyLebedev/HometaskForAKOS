#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>
#include <regex.h>

static const int ARGUMENT_ERROR_EXIT_CODE = 1;
static const int MALLOC_ERROR_EXIT_CODE = 2;
static const int WRONG_REGEX_EXIT_CODE = 3;
static char *const ERROR_PREFIX = "find: Error in file \"";
static char *const ERROR_INFIX = "\"";


int checkNull(void *pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Wrong memory alloc.\n");
        return 1;
    } else
        return 0;
}

void handleFinding(char **exec, size_t execSize, char *filename) {
    if (exec) {
        int child = fork();
        if (child < 0) {
            perror("Can't launch process");
        } else {
            if (child) {
                int status;
                do {
                    waitpid(child, &status, 0);
                } while (!WIFEXITED(status));
                int code = WEXITSTATUS(status);
                if (code) {
                    fprintf(stderr, "find: exec on \"%s\" failed, exit code %d\n", filename, code);
                } else {
                    printf("%s\n", filename);
                }
            } else {
                for (int i = 0; i < execSize; ++i) {
                    if (!strcmp(exec[i], "{}")) {
                        exec[i] = filename;
                    }
                }
                if (execvp(exec[0], exec)) {
                    perror("Can't launch child");
                }
            }
        }
    } else {
        printf("%s\n", filename);
    }
}

void printError(char *message) {
    fprintf(stderr, "find: %s\n", message);
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

void perrorFile(const char *name) {
    int code = errno;
    char *message = strconcat(3, ERROR_PREFIX, name, ERROR_INFIX);
    if (!checkNull(message)) {
        errno = code;
        perror(message);
        free(message);
    }
}

void searchRegex(regex_t *regex, char *path, char **exec, size_t execSize) {
    struct stat information;
    char *oldpath = path;
    path = strlen(path) ? path : "/";
    if (lstat(path, &information)) {
        perrorFile(path);
    } else {
        if (regex) {
            size_t pathLength = strlen(path);
            char *name = path + pathLength;
            while ((path < name) && ((*name) != '/'))
                --name;
            if ((*name) == '/')
                ++name;
            pathLength -= (name - path);
            size_t nmatch = pathLength + 1;
            regmatch_t pmatch[nmatch];
            if (!regexec(regex, name, nmatch, pmatch, 0)) {
                for (int i = 0; pmatch[i].rm_so != -1; i++) {
                    if ((pmatch[i].rm_so == 0) && pmatch[i].rm_eo == pathLength)
                        handleFinding(exec, execSize, path);
                }
            }
        } else {
            handleFinding(exec, execSize, path);
        }
        if (S_ISDIR(information.st_mode) && (!S_ISLNK(information.st_mode))) {
            DIR *directory = opendir(path);
            if (directory) {
                struct dirent *entry;
                while (entry = readdir(directory)) {
                    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
                        char *filename = strconcat(3, oldpath, "/", entry->d_name);
                        if (checkNull(filename)) {
                            fprintf(stderr, "find: malloc error in handling \"%s\" in \"%s\"", entry->d_name, path);
                        }
                        searchRegex(regex, filename, exec, execSize);
                        free(filename);
                    }

                }
                if (closedir(directory)) {
                    perrorFile(path);
                }
            } else
                perrorFile(path);
        }
    }
}

char *makeregex(char *mask) {
    if (!mask)
        return NULL;
    size_t length = strlen(mask);
    size_t originLength = length;
    int slashInBegin = 0;
    int slashInEnd = 0;
    if (mask[length - 1] == '$') {
        slashInEnd = 1;
        ++length;
    }
    if (mask[0] == '^') {
        slashInBegin = 1;
        ++length;
    }
    for (int i = 0; i < originLength; ++i) {
        if ((mask[i] == '*') || (mask[i] == '.'))
            ++length;
    }
    char *regex = malloc((length + 1) * sizeof(char));
    if (checkNull(regex)) {
        exit(MALLOC_ERROR_EXIT_CODE);
    }
    size_t index = 0;
    if (slashInBegin) {
        regex[index++] = '\\';
    }
    for (int j = 0; j < originLength; ++j) {
        switch (mask[j]) {
            case '*':
                regex[index++] = '.';
                regex[index++] = '*';
                break;
            case '.':
                regex[index++] = '\\';
                regex[index++] = '.';
                break;
            case '?':
                regex[index++] = '.';
                break;
            default:
                regex[index++] = mask[j];
                break;
        }
    }
    if (slashInEnd) {
        mask[index] = mask[index - 1];
        mask[index - 1] = '\\';
        ++index;
    }
    regex[index] = 0;
    return regex;
}

void search(char *name, char *path, char **exec, size_t execSize) {
    int pathLength = strlen(path);
    char *newpath = malloc((pathLength + 1) * sizeof(char));
    if (checkNull(newpath))
        exit(MALLOC_ERROR_EXIT_CODE);
    for (int i = 0; i <= pathLength; ++i) {
        newpath[i] = path[i];
    }
    if (newpath[pathLength - 1] == '/')
        newpath[pathLength - 1] = 0;
    char *regex = makeregex(name);
    if (regex) {
        regex_t mask;
        if (regcomp(&mask, regex, 0)) {
            perror("Broken regex");
            exit(WRONG_REGEX_EXIT_CODE);
        }
        searchRegex(&mask, newpath, exec, execSize);
        regfree(&mask);
        free(regex);
    } else
        searchRegex(NULL, newpath, exec, execSize);
    free(newpath);
}

int main(int argc, char **argv) {
    char *name = NULL;
    char *path = NULL;
    char **exec = NULL;
    size_t execSize = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-name")) {
            if ((i + 1) < argc) {
                if (name) {
                    printError("double -name argument");
                    return ARGUMENT_ERROR_EXIT_CODE;
                }
                name = argv[i + 1];
                ++i;
            } else {
                printError("there is no argument after -name");
                return ARGUMENT_ERROR_EXIT_CODE;
            }
            continue;
        }
        if (!strcmp(argv[i], "-exec")) {
            if (exec) {
                printError("double -exec argument");
                return ARGUMENT_ERROR_EXIT_CODE;
            }
            for (int j = 1; (i + j < argc) && (!exec); ++j) {
                if (!strcmp(argv[i + j], ";")) {
                    execSize = j - 1;
                    exec = malloc((execSize + 1) * sizeof(char *));
                    if (checkNull(exec)) {
                        return MALLOC_ERROR_EXIT_CODE;
                    }
                    for (int copyIndex = 1; copyIndex < j; ++copyIndex) {
                        exec[copyIndex - 1] = argv[i + copyIndex];
                    }
                    exec[execSize] = NULL;
                    i += j;
                }
            }
            if (!(exec && execSize)) {
                printError("there is no complete argument after exec");
                return ARGUMENT_ERROR_EXIT_CODE;
            }
            continue;
        }
        if (name || path || exec) {
            printError("there is unknown arguments");
        } else {
            path = argv[i];
        }
    }
    if (!path)
        path = ".";
    search(name, path, exec, execSize);
    if (exec)
        free(exec);
    return 0;
}