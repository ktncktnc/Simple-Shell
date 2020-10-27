#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 


#define MAX_LENGHT 100
#define MAX_HISTORY 10
const char exitCode[] = "exit";
const char historyCode[] = "!!";

//Print current directory.
void Dir(){
    printf("osh>");
    return;
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
//      char *argsPipe[]: array to get splitted piped args.
//Return: 
//      0: normal case.
//      1: piped case.
bool splitString(char *str, char* args[], char* argsPipe[]){
    return 1;
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
void historyProceed(int& historyCount, char* history[], char* inputStr){
    inputStr = history[historyCount-1];
    Dir();
    printf("%s", inputStr);
}

//Use parsed args to run normal command.
//Args:.
//      char* args[]: array of args to be execute.
void runCommand(char* args[]){
    return;
}

//Use parsed args to run piped command.
//Args:
//      char* args[]: array of args to be execute.
//      char* argsPipe[]: array of piped args.
void runPipedCommand(char* args[], char* argsPipe[]){
    return;
}

int main(){
    //Array to store inputted string, args and piped args.
    char inputStr[MAX_LENGHT], *args[MAX_LENGHT], *argsPipe[MAX_LENGHT];

    //Flag of normal command or piped command.
    bool flag = 0;

    //Flag of input status.
    int inputFlag = 1; 

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
            if(historyCount == 0) continue;
            else{
                historyProceed(historyCount, history, inputStr);
            }
        }
        else{
            addHistory(historyCount, history, inputStr);
        }

        //Split inputted string into args.
        flag = splitString(inputStr, args, argsPipe);

        //Normal command
        if(flag == 0)
            runCommand(args);
            
        //Piped command.
        else if(flag == 1)
            runPipedCommand(args, argsPipe);
    }
    for (int i = 0; i < historyCount; i++) 
        free(history[i]);
    return 0;

}