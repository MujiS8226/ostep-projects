// find if a match is found
        // case 1: simple test - find one line out of three in the file
        // case 2: file does not exist
        // case 3: too few command-line arguments (zero)
        // case 4: standard input test
        // case 5: long search term
        // case 6: long lines of text
        // case 7: long file test (with few matches)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *pattern = argv[1];
    char line[1024];
    // if no argument is specified, exit with code 1
    if (argc < 2) {
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }
    // stdin: cat tests/4.in | ./wgrep words
    // check if no file is specified and there are standard output in the stream
    if (argc == 2 && fgets(line, sizeof(line), stdin)) {
        while (fgets(line, sizeof(line), stdin)) {
            if (strstr(line, argv[1])) { // looked this function up
                printf("%s", line);
            }
        }
        return 0;
    }
    
    FILE *fp = fopen(argv[2], "r");
    // if file cannot be opened, exit with code 1
    if (fp == NULL) {
        printf("wgrep: cannot open file\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, pattern)) {
            printf("%s", line);
        }
    }

    fclose(fp);

    return 0;
}