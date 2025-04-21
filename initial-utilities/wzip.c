#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helper function to read and compress
void compress_file(FILE *fp, char *prev, int *count) {
    char line[1024];
    // *** use pointer to track the characters!
    while (fgets(line, sizeof(line), fp)) {
        int len = strlen(line); // check length of line instead of whole array
        for (int i = 0; i < len; i++) {
            char c = line[i];
            if (c == *prev) {
                (*count)++;
            } else {
                if (*count > 0) { // fwrite writes the count and char in outstream
                    fwrite(count, sizeof(*count), 1, stdout); 
                    fputc(*prev, stdout);
                }
                *prev = c; // include new char
                *count = 1;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // if no argument is specified, exit with code 1
    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }
    // initialize variables here to persist across files 
    char prev = EOF;
    int count = 0;

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        // if file cannot be opened, exit with code 1
        if (fp == NULL) {
            printf("wzip: cannot open file\n");
            exit(1);
        }

        compress_file(fp, &prev, &count);
        fclose(fp);
    }

    // remaining char
    if (count > 0) {
        fwrite(&count, sizeof(count), 1, stdout);
        fputc(prev, stdout);
    }

    return 0;
}
