#include <stdio.h>
#include <unistd.h>

#define MAX_LENGHT 100

//Print current directory.
void Dir();

//Get input from stdin
//Args:
//      char *str: a char* var to store inputted string.
//Return:
//      0: normal case
//      1: empty case.
//      -1: exit case.
int input(char* str);


//Split inputted String into args.
//Args:
//      char *str: inputted string
//      char *args[]: array to get splitted args.
//      char *argsPipe[]: array to get splitted piped args.
//Return: 
//      0: normal case.
//      1: piped case.
bool splitString(char *str, char* args[], char* argsPipe[]);

//Use parsed args to run normal command.
//Args:
//      char* args[]: array of args to be execute.
void runCommand(char* args[]);

//Use parsed args to run piped command.
//Args:
//      char* args[]: array of args to be execute.
//      char* argsPipe[]: array of piped args.
void runPipedCommand(char* args[], char* argsPipe[]);


int main(){
    //Array to store inputted string, args and piped args.
    char inputStr[MAX_LENGHT], *args[MAX_LENGHT], *argsPipe[MAX_LENGHT];

    //Flag of normal command or piped command.
    bool flag = 0;

    //Flag of input status.
    int inputFlag = 1; 

    while(1){
        
        //Print current dir
        Dir();

        //Get string from stdin.
        inputFlag = input(inputStr);

        //If nothing was inputted: go to new loop.
        if(inputFlag == 1) continue;

        //If user entered "Exit": exit
        else if(inputFlag == -1) break;

        //Split inputted string into args.
        flag = splitString(inputStr, args, argsPipe);

        //Normal command
        if(flag == 0)
            runCommand(args);
            
        //Piped command.
        else if(flag == 1)
            runPipedCommand(args, argsPipe);
    }
    return 0;

}