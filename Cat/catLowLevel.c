#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    size_t size = 8192;
    char *text = malloc(size* sizeof(char));
    if (text == NULL) {
        fprintf(stderr, "Error in malloc\n");
    } else {
        for (int i = 1; i < argc; ++i) {
            int file = open(argv[i], O_RDONLY);
            if (file >= 0) {
                ssize_t count = 0;
                do {
                    count = read(file, text, size);
                    if (count < 0) {
                        perror(argv[i]);
                    }
                    else {
                        write(STDOUT_FILENO, text, count);
                    }
                } while (count > 0);
                close(file);
            } else {
                perror(argv[i]);
            }
        }
        free(text);
    }
    return 0;
}
