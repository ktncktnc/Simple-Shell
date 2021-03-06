#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>
#include <iostream>

#define MAX_LENGHT 100
#define MAX_ARG_SIZE 20
#define MAX_HISTORY 10
const char exitCode[] = "exit";
const char historyCode[] = "!!";
const char pipeCode = '|';
const char backgroundCode = '&';
const char inputRe = '<';
const char outputRe = '>';

//Print current directory.
void Dir(){
    printf("osh>");
    return;
}

//Get current time in format: min/hour - date/month
char *getTime(){
    time_t my_time;
    struct tm * timeinfo; 
    time (&my_time);
    timeinfo = localtime (&my_time);
    char * output;
    sprintf(output, "%d:%d - %d/%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_mday, timeinfo->tm_mon + 1);
    return output;
}

//Get input from stdin
//Args:
//      char *str: a char* var to store inputted string.
//Return:
//      0: normal case
//      1: empty case.
//      -1: exit case.
int input(char* str){
    fflush(stdin);
    fgets(str, MAX_LENGHT, stdin);
    str[strlen(str) - 1] = '\0';
    if(strlen(str) == 0)
        return 1;
    else if(strcmp(str, exitCode) == 0) return -1;
    return 0;
}


//Split inputted String into args.
//Args:
//      char *str: inputted string
//      char *args[]: array to get splitted args.
void parseArgsCommand(char *str, char* args[], bool& backgroundCase){
    for(int i = 0; i < MAX_ARG_SIZE; i++){
        args[i] = NULL;
    }

    if(str[strlen(str) - 1] == backgroundCode){
        backgroundCase = 1;
        str[strlen(str) - 1] = '\0';
    }
    else
    {
        backgroundCase = 0;
    }
    
    int idx = 0;
    char* tok;
    tok = strtok(str, " ");
    while(tok != NULL){
        args[idx] =  tok;
        tok = strtok(NULL, " "); 
        idx++;
    }
}


//Split inputted string in case redirecting input/ ouput.
//Args:
//      args: splitted string from parseArgsCommand function.
//      redirect: store file name.
//Return:
//      0: not a input/ ouput redirecting case.
//      1: input redirecting case.
//      2: output redirecting case.
int parseRedirect(char* args[], char*& redirect){
    int idx = 0;
    while(args[idx] != NULL){
        if(args[idx][0] == inputRe){
            if(args[idx + 1] != NULL){
                redirect = args[idx + 1];
                args[idx + 1] = NULL;
                args[idx] = NULL;
                return 1;
            }
        }
        else if(args[idx][0] == outputRe){
            if(args[idx + 1] != NULL){
                redirect =  args[idx + 1];
                args[idx + 1] = NULL;
                args[idx] = NULL;
                return 2;
            }
        }
        idx++;
    }
    return 0;
}

//Parse string in case pipe.
//Args:
//      args: splitted string from parseArgsCommand function.
//      argsPipe[0]: store args before pipeCode(|).
//      argsPipe[1]: store args after pipeCode(|).
//Returns:
//      0: normal case.
//      1: pipe case.
bool parsePipe(char* args[], char* argsPipe[2][MAX_ARG_SIZE]){
    int idx = 0;
    for(int i = 0; i < MAX_ARG_SIZE; i++){
        argsPipe[0][i] = NULL;
        argsPipe[1][i] = NULL;
    }
    while(args[idx] != NULL){
        if(args[idx][0] == pipeCode){
            for(int i = 0; i < idx; i++){
                argsPipe[0][i] =  args[i];
            }
            for(int i = idx + 1; i < MAX_ARG_SIZE; i++){
                if(args[i] == NULL) break;
                argsPipe[1][i - idx - 1] = args[i];
            }
            return 1;
        }
        idx++;
    }
    return 0;
}

//Add inputted string to history.
//Args:
//      int historyCount: amount of history commands.
//      char* history[]: store history commands.
void addHistory(int& historyCount, char* history[], char* str){
    if(historyCount < MAX_HISTORY){
        strcpy(history[historyCount], str);
        historyCount++;
    }
    else{
        free(history[0]);
        for(int i = 0; i < historyCount - 1; i++) history[i] = history[i+1];
        strcpy(history[historyCount - 1], str);
    }
}


//Process inputted string when the case !! happened.
//Args:
//      int& historyCount: number of history
//      char* history[]: array to store history commands.
//      char* inputStr: var to be assigned history string.
void historyProceed(int& historyCount, char* history[], char inputStr[]){
    strcpy(inputStr, history[historyCount-1]);
    Dir();
    printf("%s\n", inputStr);
}

//Use parsed args to run normal command.
//Args:.
//      char* args[]: array of args to be execute.
void runCommand(char* args[], char* redirect, int redirectFlag){
    if(redirectFlag == 1){
        int inputFile = open(redirect, S_IRUSR);
        if(inputFile == -1){
            perror("File is not found");
            exit(EXIT_FAILURE);

        }
        int _fd = dup2(inputFile, fileno(stdin));

        if(close(inputFile) == -1){
            perror("Closing input file error");
            exit(EXIT_FAILURE);
        }

    }
    else if(redirectFlag == 2){
        int outputFile = open(redirect,  O_WRONLY| O_APPEND );
        if(outputFile == -1){
            outputFile = creat(redirect, S_IRWXU);
            if(outputFile == -1){
                perror("File is not found");
                exit(EXIT_FAILURE);
            }
        }
        dup2(outputFile, fileno(stdout));
        printf("\nRunning command %s at %s\n", args[0], getTime());
       
        if(close(outputFile) == -1){
            perror("Closing output file error");
            exit(EXIT_FAILURE);
        }
    }
    if(execvp(args[0], args) == -1){
        perror("Command not found");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);

    
}

//Use parsed args to run piped command.
//Args:
//      char* args[]: array of args to be execute.  
//      char* argsPipe[]: array of piped args.
void runPipedCommand(char* argsPipe[2][MAX_ARG_SIZE]){
    int pipefds[2];
    //pipefds[1]: write
    //pipefds[0]: read
    if(pipe(pipefds) == -1){
        /*
            Pipe parameters :
            fd[0] will be the fd(file descriptor) for the read end of pipe.
            fd[1] will be the fd for the write end of pipe.
            Returns : 0 on Success and -1 on error.
        */
        perror("Pipe error");
        exit(EXIT_FAILURE);
    }

    pid_t status = fork(), pid;
    if(status < 0){
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    else if(status == 0){
        /* 
            dup2(pipefds[1], fileno(stdout));
           All the printf statements will be written in the file pipefds[1]
        */
        dup2(pipefds[1], fileno(stdout));
        close(pipefds[0]);
        close(pipefds[1]);
        /*
            execvp: execute argsPipe[0] 
        */
        if(execvp(argsPipe[0][0], argsPipe[0]) == -1){
            perror("Fail to run first command");
            exit(EXIT_FAILURE);
        }
    }

    status = fork();
    if(status < 0){
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    else if(status == 0){
        dup2(pipefds[0], fileno(stdin));
        close(pipefds[0]);
        close(pipefds[1]);
        if(execvp(argsPipe[1][0], argsPipe[1]) == -1){
            perror("Fail to run second command");
            exit(EXIT_FAILURE);
        }
    }

    close(pipefds[0]);
    close(pipefds[1]);
    
    //Wait for all child process to terminate.
    wait(NULL);
    wait(NULL);
}

int main(){
    //Array to store inputted string, args and piped args, filename
    char inputStr[MAX_LENGHT], *args[MAX_ARG_SIZE], *argsPipe[2][MAX_ARG_SIZE], *redirect;

    //Flag of normal command or piped command.
    bool pipeFlag = 0, backgroundFlag = 0;

    //Flag of input status.
    int inputFlag = 1, redirectFlag = 0;

    //History variable.
    int historyCount = 0;
    char* history[MAX_HISTORY];
    for (int i = 0; i < MAX_HISTORY; i++) 
        history[i] = (char*)malloc(MAX_LENGHT);

    while(1){
        
        //Print current dir
        Dir();

        //Get string from stdin.
        inputFlag = input(inputStr);

        //If nothing was inputted: go to new loop.
        if(inputFlag == 1) continue;

        //If user entered "Exit": exit
        else if(inputFlag == -1) break;

        if(!strcmp(inputStr, historyCode)){
            if(historyCount == 0) {
                std::cerr << "No commands in history\n";
                continue;
            }
            else{
                historyProceed(historyCount, history, inputStr);
            }
        }
        else{
            addHistory(historyCount, history, inputStr);
        }

        //Split inputted string into args.
        parseArgsCommand(inputStr, args, backgroundFlag);

        redirectFlag = parseRedirect(args, redirect); 
        //redirectFlag = 1: input
        //redirectFlag = 2: output
        //redirectFlag = 0: not in 1 and 2
        pipeFlag = 0;

        if(redirectFlag == 0)
            pipeFlag = parsePipe(args, argsPipe);

        //pipeFlag = 1: Pipe case.
        //PipeFlag = 0:
        //  redirectFlag = 1: input redirecting case.
        //  redirectFlag = 2: output redirecting case.
        //  redirectFlag = 0: normal case.
        //      backgroundFlag = 1: concurrent case.
        //      backgroundFlag = 0: normal case.
        
        if(pipeFlag == 1)
            runPipedCommand(argsPipe);
        else{
            pid_t curPid = fork();
            if(curPid == -1){
                perror("Fork error");
                exit(EXIT_FAILURE);
            }
            else if(curPid == 0){
                runCommand(args, redirect, redirectFlag);
            }
            else{
                if(backgroundFlag == 0){
                    int stat_loc;
                    waitpid(curPid, &stat_loc, 0);
                }
            }
        }
        
    }
    for (int i = 0; i < historyCount; i++) 
        free(history[i]);
    return 0;

}
