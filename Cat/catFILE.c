#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        FILE *file = fopen(argv[i], "r");
        if (ferror(file) || (file == NULL)) {
            perror(argv[i]);
        } else {
            size_t count = 0;
            size_t size = 1024;
            char *text = malloc(size * sizeof(char));
            if (text == NULL) {
                fprintf(stderr, "Error in malloc\n");
            } else {
                do {
                    count = fread(text, sizeof(char), size, file);
                    for (int j = 0; j < count; ++j) {
                        printf("%c", text[j]);
                    }
                    if(ferror(file)){
                        perror(argv[i]);
                        break;
                    }
                } while (count > 0);
                free(text);
                fclose(file);
            }
        }
    }
    return 0;
}
