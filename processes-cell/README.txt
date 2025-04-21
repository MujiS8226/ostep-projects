# README file
Idea: when you type in a command (in response to its prompt), the shell creates a child process that executes the command, then prompts for more user input when it has finished
Implementation: Wisconsin Shell is an interactive loop: it repeatedly prints a prompt wish>  , parses the input, executes the command specified on that line of input, and waits for the command to finish. This is repeated until the user types exit.
The shell can be invoked with 0 / 1 argument; interactive mode allows the user to type commands directly, batch mode reads input from a batch file and executes commands from therein
Milestones:
[V] Command parsing: separate the input by spaces & trim
[V] Built-in commands: exit, cd, and path.
[V] External commands: fork(), exec(), and wait()/waitpid()
[V] Redirection: Handle output redirection with >.
[V] Parallel execution: Run commands in background with &.



# Preparation
1. Read the READ.md and project description in WhiteSpace; review the lecture recordings.
2. Search up the man pages and user cases of recommended functions.
3. Review inclass.c.

# Processes
1. Build up the basic structure based on inclass.c -> while loop with getline to receive the input, create child process to run the function separately.
2. Parse the input line and trim whitespaces -> create a while loop that separate each substring using strsep, shrink the size of the array until there are no leading or trailing spaces, then return the substring
    2a. Frequent -> build a funtcion parse_line()
    2b. getline find eof / end of command -> make error conditions for eof (success) / -1 (failure) / empty line (break loop)
3. Build built-in commands -> simple exit; for path, clear the current paths in the list and input new paths; for cd, chdir(path)
    3a. Exit not working -> V
        3ai. threw the function inside child process, so it simply killed the child process and back to loop -> move exit outside to check before fork() V
        3aii. builtin_exit does not work -> it return 0 when I need it to kill the program -> instead of return 0, makes the function run exit(status) V
    3b. cd not working (test1) -> issues with bacth mode (7)
        3bi. hard to keep track -> combine builtin functions & move out as a function is_builtin()
    3c. paths overwrite -> clear old paths into NULL & write duplicates of new paths into list V
    3d. does not work after implementing redirection -> check if the thread is ridirection before checkinf if it is a built-in command, and cut the args right before > -> handle cases of built-in function + redirection
4. Handle external commands by forking processes -> face issue on executing commands, found the issue by leaving comments in between loops, which is having empty strings in args, so omit empty strings during parsing.
    4a. not recognizing the external commands -> V
        4ai. ls: : No such file or directory -> adjust the parsing so that empty tokens are ommitted and the last element in args is NULL (null-terminated array) -> trim leading spaces for parse line with parallel commands V
        4aii. execv() fail -> concatenate the path using snprintf V
    4b. check if executable -> separate as function is_executable() V
    4c. not working after implementing redirection -> detect empty parse using '\0' instead of "\0" V 
5. Handle redirection so that the ouput of command is written in the file. First check if there exists > using strstr(), then check if the format is correct (cmd > output)
    5a. loop through args and find the index where > is at, and find if the format is correct (command (optional) > output) V
        5ai. use ri for redirection index, make it -1 so that no indices have the same value
    5b. Write in file -> use the long open() V
6. Build parallel commands by using the parsing to store commands in array and create threads. For each individual command, build a thread and function mythread() to handle thread executions. Use pthread_create & pthread_join to create thread objects & combine them after the threads are done. 
    6a. Build Thread class -> thread pid & args for array of commands V
        6ai. t->args = (null) -> check thread object -> command (first word) is omitted during parsing -> 
        6aii. segmentation fault -> cannot printf("%n%s\n", cmdi, commands[cmdi]);
    6b. pthread_create need argument start_routine() -> V
        5bi. move the fork() into mythread(), and build redirection inside 
        5bii. stop when return -> return NULL when builtin commands / errors / end process
    6c. wish> does not respond to command & simply loops over -> V
        6ci. connects back to 6ai.: pthread_create() always return NULL because the parsing omits the command and args becomes empty 
        6cii. combine parallel parsing to be a version of function parse_line() with switch to decide the delimiter to be &
7. Build batch mode which have the file to read commands in as an extra parameter
    7a. The only difference between the 2 modes is the way the program receives commands. In batch mode, the input stream is the file. And in batch mode it does not print wish> , simply execute the lines & exit
8. Debug based on the results of the tests
    1. bad cd
        1a. output incorrect: when cd has no arguments return 0 
        1b. error incorrect: no print out
    2. 2 arguments are passed to cd
        2a. err incorrect: it shouldn't run the first file -> cannot set cd error return 0, else the process will treat it like an external command -> treat the return value of is_builtin() as a handler to not find if the process is success/failure (error) but whether builtin or not
    7. Set path, run a shell script. Overwrite path and then try running the script again.
        7a. type the commands manually in interactive mode: lack "/" in between path and file -> snprintf "/" in between
    11. Normal redirection.
        11a. out incorrect: there are no spaces before / after >, so it was not parsed -> add an extra function to check if a line has >, and add spaces to it before parsing
    12. Input to check bad redirection. Contains no command before '>'.
        12a. rc incorrect: handle another case where > is the first argument in args
    20.  Redirection and Parallel commands combined
        20a. out incorrect: extra tests at the end -> forgot to comment out debugging printf
Done!

# Reference websites
1. StackOverflow
2. ChatGPT
3. Angelo's assistance
