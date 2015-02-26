#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        FILE *file = fopen(argv[i], "r");
        if (file != NULL) {
            size_t count = 0;
            size_t size = 100;
            char *text = malloc(size * sizeof(char));
            if (text == NULL) {
                fprintf(stderr, "Error in malloc\n");
            } else {
                do {
                    count = fread(text, sizeof(char), size, file);
                    for (int j = 0; j < count; ++j) {
                        printf("%c", text[j]);
                    }
                } while (count > 0);
                free(text);
                fclose(file);
            }
        } else {
            fprintf(stderr, "Can't open file '%s' with error %d\n", argv[i], errno);
        }
    }
    return 0;
}
