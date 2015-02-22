#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int checkNull(char *pointer) {
    if (pointer == NULL) {
        printf("Wrong memory alloc.\n");
        return 1;
    } else
        return 0;
}

int errorCode = 0;
int currentSymbol = 0;

void readSymbol() {
    currentSymbol = getchar();
}

char *readToken(char **lexemes, size_t lexemesCount) {
    int inQuotes = 0;
    size_t capacity = 10;
    size_t length = 0;
    char *line = (char *) malloc(capacity * sizeof(char));
    if (checkNull(line)) {
        errorCode = 1;
        return NULL;
    }
    line[0] = 0;
    if (currentSymbol == 0) {
        readSymbol();
    }
    while (isspace(currentSymbol)) {
        readSymbol();
    }
    if (currentSymbol == EOF) {
        return NULL;
    }
    int inWord = 1;
    do {
        if ((!inQuotes) && (currentSymbol == '"' || currentSymbol == '\'')) {
            inQuotes = currentSymbol;
        } else {
            if (inQuotes && (currentSymbol == inQuotes)) {
                inQuotes = 0;
            } else {
                line[length] = (char) currentSymbol;
                ++length;
                if (length == capacity) {
                    capacity *= 2;
                    line = (char *) realloc(line, capacity * sizeof(char));
                    if (checkNull(line)) {
                        errorCode = 1;
                        return NULL;
                    }
                }
                line[length] = 0;
            }
        }
        readSymbol();
        if (currentSymbol == EOF) {
            if (inQuotes) {
                errorCode = 2;
                printf("Wrong quote structure.\n");
                return NULL;
            } else {
                inWord = 0;
            }
        } else {
            if ((!inQuotes) && isspace(currentSymbol)) {
                inWord = 0;
            }
        }
    } while (inWord);
    return line;
}

size_t listSize = 0;
size_t listCapacity = 10;
char **list;

int addString(char *line) {
    if (listSize == listCapacity) {
        listCapacity *= 2;
        list = realloc(list, listCapacity * sizeof(char *));
        if (checkNull(line)) {
            return 1;
        }
    }
    list[listSize++] = line;
    return 0;
}

int cmpstring(const void *first, const void *second)
{
    return strcmp(* (char * const *) first, * (char * const *) second);
}


int main() {
    list = malloc(listCapacity * sizeof(char *));
    char *currentLine;
    do {
        currentLine = readToken(NULL, 0);
        if (currentLine) {
            if (addString(currentLine)) {
                errorCode = 3;
                currentLine = NULL;
            }
        }
    } while (currentLine);
    if (!errorCode) {
        qsort(list, listSize, sizeof(char*), cmpstring);
        for (int i = 0; i < listSize; ++i) {
            printf("\"%s\" \n", list[i]);
        }
    }
    for (int i = 0; i < listSize; ++i) {
        free(list[i]);
    }
    free(list);
    return errorCode;
}