// print out content of the files
        // case 1: single file
        // case 2: multiple files
        // case 3: empty file
        // case 4: no input file at all
        // case 5: long file
        // case 6: bad file on command line (does not exist)
        // case 7: many files on command line, but one of them does not exist
        
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char line[1024];
    // if no file is specified, exit with code 0
    if (argc == 1) {
        //printf("Parameter cannot be empty\n");
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        // if file cannot be opened, exit with code 1
        if (fp == NULL) {
            printf("wcat: cannot open file\n");
            exit(1);
        }
        else {
            // switch to check empty file (pass fopen but not fgets) 
            int isEmpty = 0;
            while (fgets(line, sizeof(line), fp)) {
                printf("%s", line);
                isEmpty = 1;
            }
            if (!isEmpty) {
                // printf("File cannot be empty\n");
                exit(0);
            }
        }
        fclose(fp);
    }

    return 0;
}