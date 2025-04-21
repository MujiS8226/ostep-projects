// decompress the zip file

#include <stdio.h>
#include <stdlib.h>

// helper function to 
void decompress_file(FILE *fp) {
    int count;
    char c;

    while (fread(&count, sizeof(int), 1, fp) == 1) { // fread return number of objects red
        fread(&c, sizeof(char), 1, fp);
        for (int i = 0; i < count; i++) {
            printf("%c", c);
        }
    }
}

int main(int argc, char *argv[]) {
    // if no argument is specified, exit with code 1
    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "rb"); // open non-text file
        if (!fp) {
            printf("wunzip: cannot open file\n");
            exit(1);
        }
        decompress_file(fp);
        fclose(fp);
    }

    return 0;
}
