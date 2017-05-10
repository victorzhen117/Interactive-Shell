#include "sfish.h"
#include "debug.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

/*
 * As in previous hws the main function must be in its own file!
 */

pid_t Fork();
int Dup2(int, int);
int hasASlash(char*);
char** parsethiscmd();
int getArgumentSize(char**);
int Pipe(int pipefd[2]);
char* getWorkingDirectoryPipe(char*);
int findArgument(char**, char*);
int areThereTwoPipes(char**);
int parseString(char*);
void sigAlarmHandler(int);
void sigUser2Handler(int);
void sigChldHandler(int, siginfo_t*, void*);
void cpuUtilTest();
int timer = 0;

int main(int argc, char const *argv[], char* envp[]){
    /* DO NOT MODIFY THIS. If you do you will get a ZERO. */
    rl_catch_signals = 0;
    /* This is disable readline's default signal handlers, since you are going to install your own.*/
    char *cmd;
    char* currentDirectory = getcwd(NULL, 0);
    char* previousDirectory = getcwd(NULL, 0);

    //for sigusr2, supposed to print "well that was easy"
    Signal(SIGUSR2, sigUser2Handler);
    //for sigchld timer.
    struct sigaction action;
    action.sa_sigaction = sigChldHandler;
    Sigfillset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &action, NULL);
    //more part 4 stuff, blcoking sigtstp for ctrlz.
    sigset_t mask;
    Sigemptyset(&mask);
    Sigaddset(&mask, SIGTSTP);
    Sigprocmask(SIG_BLOCK, &mask, NULL);
    //char* temp = malloc(strlen(currentDirectory)); //free temp
    //strcpy(temp, currentDirectory);
    char* startingLine = malloc(strlen(currentDirectory) + 20); // free startingline if this works.
    strcpy(startingLine, "<vzhen> : <");
    strcat(startingLine, currentDirectory);
    strcat(startingLine, "> $ ");

    while((cmd = readline(startingLine)) != NULL) {

        //parse cmd into an argv.
        char** arguments = parsethiscmd(cmd);
        if (strcmp(arguments[0], "exit")==0){
            if(arguments[1] == NULL)
                exit(1);
            else
                printf("Error.\n");
        }

        //i think this is where i do builtin functions
        //parseInformation(cmd);

        //pid_t pidPWD;

        //implement help, exit, cd, and pwd.
        /*
        if(strcmp(arguments[0], "pwd") == 0 && arguments[1] == NULL){
            if((pidPWD = Fork()) == 0){
                char* bufferPWD = getcwd(NULL ,0);
                printf("%s\n", bufferPWD);
                exit(1);
            }
            else{
                wait(NULL);
            }
        }
        else if(strcmp(arguments[0], "pwd") == 0 && arguments[1] != NULL){
            printf("Error.\n");
        }
        */

        if(strcmp(arguments[0], "help") == 0 && arguments[1] == NULL){
            printf("%-100s\n", "Here are builtin functions for this shell.");
            printf("%-50s%-50s\n", "cd [dir]", "exit");
            printf("%-50s%-50s\n", "help", "pwd");
            printf("%-50s%-50s\n", "alarm [n]", "executables [possible args]");
        }
        else if(strcmp(arguments[0], "help") == 0 && arguments[1] != NULL){
            printf("Error.\n");
        }
        char bufferCD[3] = "cd";


        //cd, get a buffer and do stuff with the arg
        if(strcmp(arguments[0], bufferCD) == 0){
            //three cases, cd - , cd ., cd..
            //do nothing in this case
            if(arguments[1] == NULL){
                free(previousDirectory);
                previousDirectory = currentDirectory;
                chdir(getenv("HOME"));
                currentDirectory = getcwd(NULL, 0);
            }
            else if(strcmp(arguments[1], ".") == 0){
                if(arguments[2] == NULL){

                }
                else{
                    printf("Error\n");
                }
                //nothing i think.
            }
            else if(strcmp(arguments[1], "..") == 0){
                if(arguments[2] == NULL){
                    free(previousDirectory);
                    previousDirectory = currentDirectory;
                    chdir("..");
                    currentDirectory = getcwd(NULL, 0);
                }
                else{
                    printf("Error\n");
                }
            }
            else if(strcmp(arguments[1], "-") == 0){
                if(arguments[2] == NULL){
                    //can i use chdir("-"); ?
                    //free(previousDirectory);
                    //memleak here
                    chdir(previousDirectory);
                    previousDirectory = currentDirectory;
                    currentDirectory = getcwd(NULL, 0);
                }
                else{
                    printf("Error\n");
                }
            }
            else{
                chdir(arguments[1]);
                currentDirectory = getcwd(NULL, 0);
            }
        }

        //alarm
        if(strcmp(arguments[0], "alarm") == 0){
            timer = parseString(arguments[1]);
            if(timer == -1){
                printf("Error.");
            }
            else{
                Signal(SIGALRM, sigAlarmHandler);
                Alarm(timer);
            }
        }

        /*
        if(strcmp(arguments[0], "xDD") == 0){
            pid_t xdd;
            if((xdd = Fork())==0) {
                cpuUtilTest();
                exit(1);
            }
            else{
                wait(NULL);
            }
        }
        */



        //now check for executables
        //if there is a slash, there are three cases: ./, /, --/--.
        //in the event of ./ or --/--, append to current directory and execve.
        if(hasASlash(arguments[0])){
            //is it a slash first?
            if(*arguments[0]=='/'){
                /*
                //go to PATH variable, append to it, and stat, execve.
                char* pathToExecs = getenv("PATH");
                arguments[0] += 1;
                char* path = malloc(strlen(pathToExecs) + strlen(arguments[0]) + 1);
                strcpy(path, pathToExecs);
                //str token now.

                const char space[2] = " ";
                char* token = strtok(cmd, space);
                strcat(path, token);

                strcat(path, arguments[0]);
                arguments[0] -= 1;
                struct stat status;
                if(stat(path, &status) < 0){
                    //that maens it doesnt exist, just continue.
                }
                else{
                    //execve here.
                    execve(path, arguments, envp);
                }
                free(path);
                */
                printf("Error\n");
            }
            else{

                int moved = 0;
                //append stuff to cwd
                if(*arguments[0]=='.'){
                    moved = 1;
                    arguments[0] += 2;
                }
                char* temp = getcwd(NULL, 0);
                char* path = malloc(strlen(temp) + strlen(arguments[0]) + 2);
                strcpy(path, temp);
                strcat(path, "/");
                //str token now.
                const char space[2] = " ";
                char* token = strtok(arguments[0], space);
                strcat(path, token);
                struct stat status;
                free(temp);
                if(stat(path, &status) < 0){
                    //that means the path doesnt exist, just exit.
                }
                else{
                    //path exists. execve it.
                    pid_t pidCommand;
                    if((pidCommand = Fork()) == 0){
                        execve(path, arguments, envp);
                        exit(1);
                    }
                    else
                        wait(NULL);
                }
                if(moved){
                    arguments[0] -= 2;
                }
                free(path);
            }
        }
        else if(strcmp(arguments[0], "pwd") == 0){
            pid_t pidPWD;
                if((pidPWD = Fork()) == 0){
                    if(arguments[1] == NULL){
                        char* bufferPWD = getcwd(NULL, 0);
                        printf("%s\n", bufferPWD);
                        exit(1);
                    }
                    if(strcmp(arguments[1], ">") == 0){
                        if(arguments[2] == NULL){
                            printf("Error.\n");
                            exit(1);
                        }
                        else{
                            int fd = open(arguments[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                            arguments[1] = NULL;
                            dup2(fd, 1);
                            char* bufferPWD = getcwd(NULL, 0);
                            printf("%s", bufferPWD);
                            exit(1);
                        }
                    }
                    /*
                    else if(strcmp(arguments[1], "<") == 0){
                        int fd = open(arguments[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                        arguments[1] = NULL;
                        dup2(fd, 0);
                    }
                    */
                    else if(strcmp(arguments[1], "|") == 0){
                        //single pipe
                        int fdArray[2];
                        pipe(fdArray);
                        pid_t pidPipe;
                        if((pidPipe = Fork()) == 0){
                            arguments[1] = NULL;
                            dup2(fdArray[1], 1);
                            close(fdArray[0]);
                            char* bufferPWD = getcwd(NULL, 0);
                            printf("%s\n", bufferPWD);
                            close(fdArray[1]);
                            exit(1);
                        }
                        else{
                            wait(NULL);
                            arguments += 2;
                            char* workingDir = getWorkingDirectoryPipe(arguments[0]);
                            dup2(fdArray[0], 0);
                            close(fdArray[1]);
                            execve(workingDir, arguments, envp);
                            close(fdArray[0]);
                            free(workingDir);
                        }
                    }
                    else{
                        printf("Error\n");
                        exit(1);
                    }
                }
                else{
                    wait(NULL);
                }
        }
        else{
            //try to find it in the path.
            char* pathToExecs = getenv("PATH");
            //now everything is separated by colons., call ls on each.
            //token time.
            char* temporaryExecPath = malloc(strlen(pathToExecs) + 1);
            strcpy(temporaryExecPath, pathToExecs);
            char* token = strtok(temporaryExecPath, ":");
            while(token != NULL){
                char* temporary = malloc(strlen(token) + strlen(arguments[0]) + 40);
                char* temporary2 = malloc(strlen(token) + strlen(arguments[0]) + 40);
                strcpy(temporary, token);
                strcat(temporary, "/");
                strcpy(temporary2, temporary); //temp2 needs to be appended with whatever.

                strcat(temporary, arguments[0]);
                struct stat status;
                //int ii = getArgumentSize(arguments);
                //printf("%d\n", ii);
                if(stat(temporary, &status) < 0){
                }
                else{
                    pid_t pidExec;
                    if((pidExec = Fork()) == 0){
                        int fd = -1;
                        int fd1 = -1;
                        int fd2 = -1;
                        int calledExecAlready = 0;
                        int i = 0;
                        int secondPipeLocation = 0;
                        for(i = 0; i< getArgumentSize(arguments)-1 ; i++){
                            if(strcmp(arguments[i], "|") == 0){
                                //from man, pipefd[0] refers to the read end of the pipe.
                                //pipefd[1] refers to the write end of the pipe
                                secondPipeLocation = areThereTwoPipes(arguments);
                                if(secondPipeLocation){
                                    //double pipe
                                    int fdArray[4];
                                    pipe(fdArray);
                                    pipe(fdArray + 2);
                                    pid_t pidPipe1;
                                    pid_t pidPipe2;
                                    pid_t pidPipe3;
                                    if((pidPipe1 = Fork()) == 0){
                                        arguments[i] = NULL;
                                        //write into stdout
                                        dup2(fdArray[1], 1);
                                        close(fdArray[0]);
                                        close(fdArray[1]);
                                        close(fdArray[2]);
                                        close(fdArray[3]);
                                        execve(temporary, arguments, envp);
                                        exit(1);
                                    }
                                    else{
                                        arguments[i] = NULL;
                                        if((pidPipe2 = Fork()) == 0){
                                            //read from stdin in first pipe, write into stdout in 2nd
                                            dup2(fdArray[0], 0);
                                            dup2(fdArray[3], 1);
                                            close(fdArray[0]);
                                            close(fdArray[1]);
                                            close(fdArray[2]);
                                            close(fdArray[3]);
                                            arguments += (i+1);
                                            secondPipeLocation = findArgument(arguments, "|");
                                            arguments[secondPipeLocation] = NULL;
                                            char* workingDir = getWorkingDirectoryPipe(arguments[0]);
                                            execve(workingDir, arguments, envp);
                                            free(workingDir);
                                            exit(1);
                                        }
                                        else{
                                            if((pidPipe3 = Fork()) == 0){
                                                //read from stdin in first pipe, and write into wherever.
                                                dup2(fdArray[2], 0);
                                                close(fdArray[0]);
                                                close(fdArray[1]);
                                                close(fdArray[2]);
                                                close(fdArray[3]);
                                                arguments += (secondPipeLocation + 1);
                                                char* workingDir2 = getWorkingDirectoryPipe(arguments[0]);
                                                execve(workingDir2, arguments, envp);
                                                free(workingDir2);
                                                exit(1);
                                            }
                                        }
                                    }
                                    close(fdArray[0]);
                                    close(fdArray[1]);
                                    close(fdArray[2]);
                                    close(fdArray[3]);
                                    int j=0;
                                    for(j=0; j<3; j++){
                                        wait(NULL);
                                    }
                                    calledExecAlready = 1;
                                }
                                else{
                                    //single pipe
                                    int fdArray[2];
                                    pipe(fdArray);
                                    pid_t pidPipe;
                                    if((pidPipe = Fork()) == 0){
                                        arguments[i] = NULL;
                                        dup2(fdArray[1], 1);
                                        close(fdArray[0]);
                                        execve(temporary, arguments, envp);
                                        close(fdArray[1]);
                                        exit(1);
                                    }
                                    else{
                                        wait(NULL);
                                        arguments += (i + 1);
                                        char* workingDir = getWorkingDirectoryPipe(arguments[0]);
                                        dup2(fdArray[0], 0);
                                        close(fdArray[1]);
                                        execve(workingDir, arguments, envp);
                                        close(fdArray[0]);
                                        free(workingDir);
                                    }
                                    calledExecAlready = 1;
                                }
                            }
                            else if(strcmp(arguments[i], "<") == 0){
                                if(findArgument(arguments, ">")){
                                    fd1 = open(arguments[i+1], O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
                                    fd2 = open(arguments[findArgument(arguments, ">") + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                                    arguments[i] = NULL;
                                    dup2(fd1, 0);
                                    dup2(fd2, 1);
                                }
                                else{
                                    fd = open(arguments[i+1], O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
                                    arguments[i] = NULL;
                                    dup2(fd, 0);
                                }
                            }
                            else if(strcmp(arguments[i], ">") == 0 || strcmp(arguments[i], "1>") == 0){
                                //post output into arg[2]
                                fd = open(arguments[i+1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                                arguments[i] = NULL;
                                dup2(fd, 1);
                            }
                            else if(strcmp(arguments[i], "2>") == 0){
                                //redirect to stderr
                                fd = open(arguments[i+1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                                arguments[i] = NULL;
                                dup2(fd, 2);
                            }
                            else if(strcmp(arguments[i], "&>") == 0){
                                //redirect to both.
                                fd = open(arguments[i+1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                                arguments[i] = NULL;
                                dup2(fd, 1);
                                dup2(fd, 2);
                            }
                            else if(strcmp(arguments[i], ">>") == 0){
                                //append to file instead of overwrite.
                                fd = open(arguments[i+1], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
                                arguments[i] = NULL;
                                dup2(fd, 1);
                            }
                            else if(strcmp(arguments[i], "<<") == 0){
                                //read stdin until delimiter.
                                //plan here is to make a char**, and determine delimiter, keep adding to char** until hit delim
                                char* delim = arguments[i+1];
                                //i use amortization to my advantage: amortized time for insertion into this buffer = O(1).
                                int currentSizeOfBuffer = 10;
                                char** strValues = malloc(sizeof(char*) * currentSizeOfBuffer);
                                int counter = 0;
                                char* command;
                                while(strcmp(command = readline("> "), delim) != 0){
                                    if(counter==currentSizeOfBuffer){
                                        currentSizeOfBuffer *= 2;
                                        strValues = (char**) realloc(strValues, sizeof(char*) * currentSizeOfBuffer);
                                    }
                                    strValues[counter++] = command;
                                }
                                //now i have a char**, i put this into input for the command.
                                int fdArray[2];
                                pipe(fdArray);
                                pid_t pidDoubleLessThanFork;
                                arguments[i] = NULL;
                                if((pidDoubleLessThanFork = Fork()) == 0){
                                    dup2(fdArray[1], 1);
                                    close(fdArray[0]);
                                    while(*strValues != NULL){
                                        printf("%s\n", *strValues);
                                        strValues += 1;
                                    }
                                    close(fdArray[1]);
                                    exit(1);
                                }
                                else{
                                    wait(NULL);
                                    dup2(fdArray[0], 0);
                                    close(fdArray[1]);
                                    execve(temporary, arguments, envp);
                                    close(fdArray[0]);
                                }
                                calledExecAlready = 1;
                                free(command);
                                free(strValues);
                            }
                        }
                        if(calledExecAlready==0)
                            execve(temporary, arguments, envp);

                        if(fd != -1){
                            close(fd);
                        }
                        if(fd1 != -1){
                            close(fd1);
                            close(fd2);
                        }
                        exit(1);
                    }
                    else{
                        wait(NULL);
                    }
                }
                token = strtok(NULL, ":");
                free(temporary);
                free(temporary2);
            }
            /*
            char* path = malloc(strlen(pathToExecs) + strlen(arguments[0]) + 2);
            strcpy(path, pathToExecs);
            strcat(path, "/");
            strcat(path, arguments[0]);
            struct stat status;
            if(stat(path, &status) < 0){
            //that maens it doesnt exist, just continue.
            }
            else{
                //execve here.
                execve(path, arguments, envp);
            }
            free(path);
            */
            free(temporaryExecPath);
        }

        /*
        printf("%s\n",cmd);
         All your debug print statements should use the macros found in debu.h
         Use the `make debug` target in the makefile to run with these enabled.
        info("Length of command entered: %ld\n", strlen(cmd));
         You WILL lose points if your shell prints out garbage values.
        */
        free(arguments);
        free(startingLine);
        startingLine = malloc(strlen(currentDirectory) + 15); // free startingline if this works.
        strcpy(startingLine, "<vzhen> : <");
        strcat(startingLine, currentDirectory);
        strcat(startingLine, "> $ ");
    }

    /* Don't forget to free allocated memory, and close file descriptors. */
    free(cmd);
    free(currentDirectory);
    free(previousDirectory);
    free(startingLine);

    return EXIT_SUCCESS;
}

int hasASlash(char* cmd){
    char* temp = cmd;
    while(*temp != '\0'){
        if(*temp == '/'){
            return 1;
        }
        temp += 1;
    }
    return 0;
}

char** parsethiscmd(char* cmd){
    char* temp = (char*) malloc(strlen(cmd) + 1);
    strcpy(temp, cmd);
    //count number of spaces
    int count = 0;
    char const delims[3] = " \t";
    char* token;
    token = strtok(temp, delims);
    while(token != NULL){
        count += 1;
        token = strtok(NULL, delims);
    }
    temp = strdup(cmd);
    char** toReturn = (char**) malloc(sizeof(char*) * (count+1));
    int counter = 1;

    token = strtok(cmd, delims);
    toReturn[0] = token;
    while(token != NULL){
        token = strtok(NULL, delims);
        toReturn[counter] = token;
        counter ++;
    }
    //free temp
    return toReturn;
}

int getArgumentSize(char** arguments){
    int i=1;
    while(*arguments){
        i++;
        arguments += 1;
    }
    return i;
}

char* getWorkingDirectoryPipe(char* argument){
    char* pathToExecs = getenv("PATH");
    //now everything is separated by colons., call ls on each.
    //token time.
    char* temporaryExecPath = malloc(strlen(pathToExecs) + 1);
    strcpy(temporaryExecPath, pathToExecs);
    char* token = strtok(temporaryExecPath, ":");
    while(token != NULL){
        char* temporary = malloc(strlen(token) + strlen(argument) + 40);
        strcpy(temporary, token);
        strcat(temporary, "/");
        strcat(temporary, argument);
        struct stat status;
        //int ii = getArgumentSize(arguments);
        if(stat(temporary, &status) < 0){
        }
        else{
            free(temporaryExecPath);
            return temporary;
        }
        free(temporary);
        token = strtok(NULL, ":");
    }
    free(temporaryExecPath);
    printf("Error");
    return NULL;
}

int findArgument(char** arguments, char* stringToBeFound){
    int i=0;
    for(i=0; i<(getArgumentSize(arguments)-1); i++){
        if(strcmp(stringToBeFound, arguments[i]) == 0)
            return i;
    }
    return 0;
}

int areThereTwoPipes(char** arguments){
    int pipeCount = 0;
    int i=0;
    for(i=0; i<(getArgumentSize(arguments)-1); i++){
        if(strcmp("|", arguments[i]) == 0 && pipeCount == 1)
            return i;
        else if(strcmp("|", arguments[i]) == 0)
            pipeCount ++;
    }
    return 0;
}

int parseString(char* argument){
    int i=0;
    int j=0;
    for(j=0; j<strlen(argument); j++){
        if(*(argument+j) < 48 || *(argument+j) > 57)
            return -1;
        i *= 10;
        i += (*(argument+j)-'0');
    }
    return i;
}

void sigAlarmHandler(int sig){
    printf("Your %i second timer has finished!\n", timer);
    timer = 0;
}

void sigUser2Handler(int sig){
    printf("Well that was easy.\n");
}

void sigChldHandler(int sig, siginfo_t *sigInfo, void* notused){
    printf("Child with PID %i has died. It spent %i milliseconds utilizing the CPU. \n",
        sigInfo->si_pid, (int)(((1000 * (double)(sigInfo->si_stime + sigInfo->si_utime)))/sysconf(_SC_CLK_TCK)));
}

void cpuUtilTest(){
    printf("Aasdffds");
    fprintf(stderr, "ASDFASDF%s\n", "asdffsda");
}
