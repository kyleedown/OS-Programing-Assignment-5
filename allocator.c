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

Command get_choice(FILE *in){
    Command result = {0};
    char buffer[256];
    

    while(1){
        printf("allocator> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), in)) {
        result.cmd = CMD_X;  // treat as exit
        return result;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        char *token = strtok(buffer, " ");
        if (!token) continue;

        result.cmd = cmd_from_string(token);
        if(result.cmd == CMD_UNKNOWN){
            printf("Invalid option. Try again\n");
            continue;
        }

        int i = 0;
        while ((token = strtok(NULL," ")) != NULL && i < MAX_ARGS){
            strncpy(result.args[i], token, MAX_TOKEN-1);
            i++;
        }
        result.argc = i;

        return result;
    }
}


int main(int argc, char *argv[]){
    int memorySize = atoi(argv[1]);
    Command c;
    FILE *input = stdin;
    if (argc > 2 && (strcmp(argv[2],"SIM")) == 0) {
        input = fopen(argv[3], "r");
        if (!input) {
            perror("fopen");
            return 1;
        }
    }

    char *pid;
    int size;
    char* fit_type;

    while(1){
        c = get_choice(input);
        switch(c.cmd){
            case CMD_RQ:
                pid = c.args[0];
                size = atoi(c.args[1]);
                fit_type = c.args[2];
                printf("PID: %s\nSize: %d\nFlag: %s\n",pid,size,fit_type);
                break;
            case CMD_RL:
                pid = c.args[0];
                break;
            case CMD_C:
                break;
            case CMD_STAT:
                break;
            case CMD_X:
                printf("exiting... ");
                return 0;
            case CMD_UNKNOWN:
                break;
        }
        
    }
    


}