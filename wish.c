#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // error message
#include <sys/wait.h>
#include <fcntl.h> //open
#include <string.h>
#include <ctype.h>
#include <pthread.h> // threads

#define BUFF_SIZE  256
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// list of directories
char *path[BUFF_SIZE] = {"/bin/", "/usr/bin/", NULL};
int paths_num = 2;

// built-in commands
void error(); // return error message
char * pre_parsing(char * line); // fix > with no leading / trailing spaces
char ** parse_line(char * line, int p); // parse input into tokens (support parallel parsing)
void * mythreads(void * arg); // handle threads
char * is_executable(char **args); // check if the external command is executable
int is_builtin(char **args); // check if the command is builtin (exit, cd, path)


// define class for threading: each command has its own pthread_t
typedef struct {
    pthread_t tpid;
    char ** args;
} Thread;

int main(int argc, char *argv[]) {
    FILE * stream;
    if (argc > 2) { // more than 2 arguments
        error();
        exit(EXIT_FAILURE);
    } else if (argc == 1) { // interactive mode: receive commands from stream
        stream = stdin;
    } else { // batch mode: read commands from file
        stream = fopen(argv[1], "r");
        if (stream == NULL) { // empty file
            error();
            exit(EXIT_FAILURE);
        }
    }
    while (1) {
        if (stream == stdin) { // only have this when interactive mode
            printf("wish> ");
        }

        // read line from input
        char * line = NULL;
        size_t len;
        // getline: one line per loop
        if (getline(&line, &len, stream) == -1) {// handle end / error
            line = NULL;
            if (feof(stream)) { // reach the end of file (not null)
                exit(EXIT_SUCCESS);
            } else { // error
                error();
                exit(EXIT_FAILURE);
            }
        }; 
        line = pre_parsing(line);
        if (line == NULL) break; // empty line
        //printf("line: %s\n", line);
        // parallel commands: separate commands into threads on &
        char ** threads = parse_line(line, 1);
        //for (int i = 0; threads[i]; i++) { // can have whitespaces -> trim later
        //    printf("threads[%d] = '%s'\n", i, threads[i]);
        //}

        // parallel commands: operate each command separately
        // create class for threads
        Thread * listthreads[BUFF_SIZE] = {0}; // build Thread object for each command
        int tcnt = 0;

        for (int ci = 0; threads[ci]; ci++) {
            Thread * t = malloc(sizeof(Thread)); // allocate memory for each thread
            t->args = parse_line(threads[ci], 0);// parse command into t->args 
            //for (int i = 0; t->args[i]; i++) { // can have whitespaces -> trim later
            //    printf("args[%d] = '%s'\n", i, t->args[i]);
            //}
            //create threads: handle executions (redirection, fork, etc.) (long!)
            //printf("threads prepared\n");
            pthread_create(&t->tpid, NULL, mythreads, (void *)t);//void for correct format
            listthreads[tcnt++] = t; // save new thread to array
        }
        // join threads together and free the memory of joined thread objects
        for (int ti=0; listthreads[ti]; ti++) {
            pthread_join(listthreads[ti]->tpid, NULL);
            free(listthreads[ti]->args); // free memory
            free(listthreads[ti]);
        }
        free(line); // free line memory at last
        //printf("finish all threads\n");
    }
    return 0;
} // end of main




void error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

char * pre_parsing(char * line) {
    char temp [BUFF_SIZE]; // copy the line letter by letter
    int j = 0;
    for (int i = 0; line[i]; i++) {
        if (line[i] == '>') { // add spaces before / after >
            temp[j++] = ' '; // no need to care about extra spaces
            temp[j++] = '>'; // parse_line will handle them
            temp[j++] = ' '; 
        } else {
            temp[j++] = line[i];
        }
    }
    temp[j] = '\0'; // end of line
    strcpy(line, temp); // fill back the fixed version to line
    return line;
}; // fix > with no leading / trailing spaces
char ** parse_line(char * line, int p) {
    char ** parses = malloc(BUFF_SIZE * sizeof(char *)); // allocate memory for parses
    char * parse;
    char * string = strdup(line);
    int parsei = 0;
    char *del;
    if (p == 1) {
        del = "&";
    } else {
        del = " \t\n"; // whitespace, tab, newline
    }
    // first word missing in line: append separately
    while ((parse = strsep(&string, del)) != NULL) {
        while (*parse == ' ' || *parse == '\t') parse++; // trim leading spaces
        // append parse into array & increment index
        if (parse[0] == '\0') continue; // skip empty parse
        parses[parsei++] = strdup(parse);
    }
    parses[parsei] = NULL;// null-terminated array
    free(string);
    return parses;
}; // parse input into tokens

char * is_executable(char **args) { // check if the external command is executable
    if (args[0] == NULL || paths_num == 0) return NULL; // no command / no path
    // look up the directory where the function is in and check if it's executable
    char complete_path[BUFF_SIZE];
    for (int i = 0; i < paths_num; i++) { 
        // concatenate the path and the command
        snprintf(complete_path, sizeof(complete_path), "%s/%s", path[i], args[0]);
        if (access(complete_path, X_OK) == 0) { // check if executable
            return strdup(complete_path); // do changes to the copy
        }
    }
    return NULL; // no executable path
}

int is_builtin(char **args) { // check if the command is builtin
    int argc = 0; // size
    while (args[argc] != NULL) argc++;
    // built-in commands: execute directly
    if (strcmp(args[0], "exit") == 0) { // kill the program if exit
        if (argc != 1) { // no arguments
            error();
            return 1;
        }
        exit(EXIT_SUCCESS); // execute exit successfully
    } else if (strcmp(args[0], "cd") == 0) {
        if (argc != 2) {// exactly 1 argument
            error();
        } else if (chdir(args[1]) != 0) {
            error(); // use chdir() to change the directory;
        } return 1;
    } else if (strcmp(args[0], "path") == 0) {
        for (int i=0; i<paths_num; i++) path[i] = NULL; // remove old path
        paths_num = 0; // reset path num
        for (int i=1; args[i]; i++) path[paths_num++] = strdup(args[i]); // new path
        // for (int i=1; path[i]; i++) printf("%s\n", path[i]);
        // printf("path overwrite success\n");
        return 1;
    }
    return 0; // not builtin
} 

void * mythreads(void * thread) {// handle threads
    Thread * t = (Thread *)thread; // set up thread
    char ** args = t->args;
    FILE * output = NULL;
    if (args[0] == NULL) return NULL; // no command
    //printf("not empty command\n");
    

    // redirection
    // use strstr() to find '&' and '>' for making threads and redirection
    // SELECTION: IF REDIRECTED CLOSE STDOUT AND OPEN NEW FILE
    int ri = -1;
    for (int i = 0; args[i]; i++) {
        if (strcmp(args[i], ">") == 0) {
            // no multiple >
            if (ri != -1) {
                error();
                return NULL;
            } else if (i == 0) { // no command before >
                error();
                return NULL;
            }
            //printf("args[%d] = '%s'\n", i, args[i]);
            ri = i; // find index of >
            //printf("redirection needed\n");
        }
    }
    if (ri != -1) {
        // check for valid format
        if (args[ri+1] == NULL || args[ri+2] != NULL) { // no output file / more than 1 file
            error();
            //printf("redirection fail\n");
            return NULL;
        } else {
        output = args[ri+1];
        args[ri] = NULL; // truncate > & output file for built-in
        args[ri+1] = NULL;
        //printf("output file prepared\n");
        }
    }
    if (is_builtin(args)) { // check if it's built-in command
        if (output != NULL) { // built-in + redirection
            int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd < 0) { // open fail
                error();
                return NULL;
            }
            // allocates a new file descriptor that refers to the same open file
            dup2(fd, STDOUT_FILENO); // redirect stdout to output file
            close(fd); // not fclose: int is not FILE *
        }
        return NULL;
    }; // test: ls -l > output.txt & echo hi
    

    // external commands
    char * executable = is_executable(args);
    if (executable == NULL) { // no executable path
        error();
        // printf("executable fail\n");
        return NULL;
    }
    // printf("executable prepared\n");

    // fork the process
    int rc = fork();
    if (rc < 0) { // fork fail
        error();
    } else if (rc == 0) { // child process
        // external commands: execute
        // redirection: save output in file
        if (output != NULL) {
            int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd < 0) { // open fail
                error();
                return NULL;
            }
            // allocates a new file descriptor that refers to the same open file
            dup2(fd, STDOUT_FILENO); // redirect stdout to output file
            close(fd); // not fclose: int is not FILE *
        }
        execv(executable, args); // take path as parameter instead of file
        error(); // will not continue if not fail
        exit(EXIT_FAILURE); // exit the child process to return back to parent
    } else { // parent process
        wait(NULL); // parallel: wait until every thread is done
        // printf("done\n");
    }
    free(executable);
    return NULL;
}