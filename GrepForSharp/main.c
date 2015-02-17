#include <stdio.h>
#include <stdlib.h>

int checkNotNull(char *pointer) {
    if (pointer == NULL) {
        printf("Wrong memory alloc.\n");
        return 1;
    } else
        return 0;
}

int main() {
    size_t length = 10;
    size_t index = 0;
    char *line = (char *) malloc(length * sizeof(char));
    if (checkNotNull(line))
        return -1;
    line[0] = 0;
    char current = 0;
    int wasSharp = 0;
    while (current != EOF) {
        current = getchar();
        if (current == EOF || current == '\n') {
            if (wasSharp)
                printf("%s\n", line);
            wasSharp = 0;
            index = 0;
            line[0] = 0;
        } else {
            line[index] = current;
            if (current == '#') {
                wasSharp = 1;
            }
            ++index;
            if (index == length) {
                length *= 2;
                line = (char *) realloc(line, length * sizeof(char));
                if (checkNotNull(line))
                    return -1;
            }
            line[index] = 0;
        }
    }
    free(line);
    return 0;
}