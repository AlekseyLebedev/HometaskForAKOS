#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    size_t size = 8192;
    char *text = malloc(size * sizeof(char));
    if (text == NULL) {
        fprintf(stderr, "Error in malloc\n");
    } else {
        for (int i = 1; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");
            if ((file == NULL) || ferror(file)) {
                perror(argv[i]);
            } else {
                size_t count = 0;
                do {
                    count = fread(text, sizeof(char), size, file);
                    if (ferror(file)) {
                        perror(argv[i]);
                        break;
                    } else {
                        fwrite(text, sizeof(char), count, stdout);
                    }
                } while (count > 0);
                fclose(file);
            }
        }
        free(text);
    }
    return 0;
}
