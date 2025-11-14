#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum{
    CMD_UNKNOWN = 0,
    CMD_RQ,
    CMD_RL,
    CMD_C,
    CMD_STAT,
    CMD_X
} CommandType;

#define MAX_ARGS 4
#define MAX_TOKEN 32

typedef struct {
    CommandType cmd;
    char args[MAX_ARGS][MAX_TOKEN];
    int argc;
} Command;

CommandType cmd_from_string(const char *s) {
    if (strcmp(s, "RQ") == 0) return CMD_RQ;
    if (strcmp(s, "RL") == 0) return CMD_RL;
    if (strcmp(s, "C") == 0) return CMD_C;
    if (strcmp(s, "STAT") == 0) return CMD_STAT;
    if (strcmp(s, "X") == 0) return CMD_X;
    return CMD_UNKNOWN;
}

Command get_choice(){
    Command result = {0};
    char buffer[256];

    while(1){
        printf("allocator> ");

        char *token = strtok(buffer, " ");
        if (!token) continue;

        result.cmd = cmd_from_string(token);
        if(result.cmd == CMD_UNKNOWN){
            printf("Invalid option. Try again");
            continue;
        }

        int i = 0;
        while ((token = strtok(NULL," ")) != NULL && i < MAX_ARGS){
            strncpy(result.args[1], token, MAX_TOKEN-1);
            i++;
        }
        result.argc = i;

        result;
    }
}


int main(int argc, char *argv[]){
    int memorySize = atoi(argv[1]);
    Command c;
    if (argc >2){
        if (strcmp(argv[2], "SIM") == 0){
            /*Using file for input*/
        }
    }
    else{ 
        /*Using Terminal for input*/
        while(1){
            c = get_choice();
            switch(c.cmd){
                case CMD_RQ:
                case CMD_RL:
                case CMD_C:
                case CMD_STAT:
                case CMD_X:
                    printf("exiting... ");
                    return 0;
                default:
                    printf("CMD complete\n");

            }
            
        }
    }
    


}