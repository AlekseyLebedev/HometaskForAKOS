#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        int file = open(argv[i], O_RDONLY);
        if (file >= 0) {
            ssize_t count = 0;
            size_t size = 100;
            char *text = malloc(size * sizeof(char));
            if (text == NULL) {
                fprintf(stderr, "Error in malloc\n");
            } else {
                do {
                    count = read(file, text, size);
                    for (ssize_t j = 0; j < count; ++j) {
                        printf("%c", text[j]);
                    }
                    if(count<0){
                        perror(argv[i]);
                    }
                } while (count > 0);
                free(text);
                close(file);
            }
        } else {
            //fprintf(stderr, "Can't open file '%s' with error %d\n", argv[i], errno);
            perror(argv[i]);
        }
    }
    return 0;
}
