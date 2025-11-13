char * get_choice(){
    char* in;
    in = input("allocator>");
    return in;

}


int main(int argc, char *argv){
    int memorySize = argv[1];
    char* in;
    if (argc >2){
        if (strcmp(argv[2], "SIM") == 0 ){
            /*Using file for input*/
        }
    }
    else{ 
        /*Using Terminal for input*/
        while (in != 'X'){
        in = get_choice();
        }
    }
    

    

    
    
    


}