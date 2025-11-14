#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int is_in_array(const char *choice, char *lst[], int list_size) {
    for (int i = 0; i < list_size; i++) {
        if (strcmp(choice, lst[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void trim(char *s) {
    // trim leading space
    while (*s == ' ') s++;

    // trim trailing space
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ')) {
        *end = '\0';
        end--;
    }
}

char * get_choice(char *lst[], int list_size){
    ssize_t read;
    char *line = NULL;
    size_t len = 0;

    printf("allocator> ");
    read = getline(&line, &len, stdin);
    if (read > 0 && line[read - 1] == '\n') {
        line[read - 1] = '\0';
        read--;
    }
    trim(line);

    while (!is_in_array(line, lst, list_size)){
            printf("Incorrect option. Try again");
            printf("allocator> ");
            read = getline(&line, &len, stdin);
            if (read > 0 && line[read - 1] == '\n') {
                line[read - 1] = '\0';
                read--;
            }
            trim(line);
        }

    return line;
}


int main(int argc, char *argv[]){
    int memorySize = atoi(argv[1]);
    char* in;
    char *choiceList[] = {"RQ","RL","C","STAT","X"};
    if (argc >2){
        if (strcmp(argv[2], "SIM") == 0){
            /*Using file for input*/
        }
    }
    else{ 
        /*Using Terminal for input*/
        do{
            in = get_choice(choiceList,5);
            printf("Your choice: %s", in);
        }
        while (strcmp(in, "X") != 0);
    }
    

    

    
    
    


}