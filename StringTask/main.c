#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

const int LEXEMES_COUNT = 5;

int checkNull(char *pointer) {
    if (pointer == NULL) {
        printf("Wrong memory alloc.\n");
        return 1;
    } else
        return 0;
}

int errorCode = 0;
int currentSymbol = 0;
int wasInLexeme = 0;
int lastWasLexeme = 0;

void readSymbol() {
    currentSymbol = getchar();
}

int checkIsBeginLexeme(char **lexemes, size_t lexemesCount) {
    wasInLexeme = 0;
    for (size_t i = 0; i < lexemesCount; ++i) {
        if (((char) currentSymbol) == lexemes[i][0]) {
            wasInLexeme = 1;
            return 0;
        }
    }
    return 1;
}

char *readToken(char **lexemes, size_t lexemesCount) {
    size_t capacity = 10;
    size_t length = 0;
    char *line = (char *) malloc(capacity * sizeof(char));
    if (checkNull(line)) {
        errorCode = 1;
        return NULL;
    }
    line[0] = 0;
    int begin = 0;
    if (currentSymbol == 0) {
        readSymbol();
        begin = 1;
    }
    while (isspace(currentSymbol)) {
        readSymbol();
    }
    if (currentSymbol == EOF) {
        return NULL;
    }
    if (begin)
        checkIsBeginLexeme(lexemes, lexemesCount);
    if (wasInLexeme) {
        if (lastWasLexeme) {
            //It have to be empty word between two lexemes.
            lastWasLexeme = 0;
        } else {
            int stillLexeme;
            do {
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
                stillLexeme = 0;
                for (size_t i = 0; i < lexemesCount; ++i) {
                    if (strcmp(line, lexemes[i]) == 0) {
                        stillLexeme = 1;
                        break;
                    }
                }
                if (stillLexeme)
                    readSymbol();
            } while (stillLexeme);
            line[length - 1] = 0;
            checkIsBeginLexeme(lexemes, lexemesCount);
            if (wasInLexeme)
                lastWasLexeme = 1;
        }
    } else {
        int inQuotes = 0;
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
                if (!inQuotes) {
                    if (isspace(currentSymbol)) {
                        inWord = 0;
                    } else {
                        inWord = checkIsBeginLexeme(lexemes, lexemesCount);
                    }
                }
            }
        } while (inWord);
    }
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

int cmpstring(const void *first, const void *second) {
    return strcmp(*(char *const *) first, *(char *const *) second);
}

void freeList(char **array, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        free(array[i]);
    }
    free(array);
}

int main() {
    char **lexemes = malloc(LEXEMES_COUNT * sizeof(char *));
    lexemes[0] = ";";
    lexemes[1] = "&";
    lexemes[2] = "&&";
    lexemes[3] = "|";
    lexemes[4] = "||";
    list = malloc(listCapacity * sizeof(char *));
    char *currentLine;
    do {
        currentLine = readToken(lexemes, LEXEMES_COUNT);
        if (currentLine) {
            if (addString(currentLine)) {
                errorCode = 3;
                currentLine = NULL;
            }
        }
    } while (currentLine);
    if (!errorCode) {
        qsort(list, listSize, sizeof(char *), cmpstring);
        for (int i = 0; i < listSize; ++i) {
            printf("\"%s\" \n", list[i]);
        }
    }
    freeList(list, listSize);
    free(lexemes);
    return errorCode;
}