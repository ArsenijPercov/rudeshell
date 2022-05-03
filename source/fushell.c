#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char **argv){
    char *input_line = NULL;
    size_t len = 0;
    ssize_t nread;

    while(1){
        printf("What the fuck do you want? > ");
        nread = getline(&input_line, &len,stdin);
        if (nread == -1){
            return EXIT_FAILURE;
        }
        char *commands[250];  
        memset((char *) commands, 0, sizeof(commands));
        
        char *path_env_var = getenv("PATH");

        char *paths[250];
        memset((char *)paths, 0, sizeof(paths));

        int counter = readParameters(input_line,&commands);


        if (strcmp(commands[0], "exit") == 0){
            exit(0);
        } else if (strcmp(commands[0], "cd") == 0){
            if (counter == 2){
                if (chdir(commands[1]) == -1){
                    int errsv = errno;
                    fprintf(stderr, "Couldn't change directory to \"%s\"\n",commands[1]);
                }
            } else {
                fprintf(stderr, "Too many parameters for cd commands.\n");
            }
        } else {
            int n_paths =  splitPathVar(path_env_var, &paths);
        
            char *cmd_w_path;
            cmd_w_path = (char*) malloc(1000);

            if (getCorrectPath(commands[0],n_paths, &paths,cmd_w_path)){
                
                int rc = fork();
                
                if (rc < 0){
                    //Fork failed
                    fprintf(stderr, "fork failed\n");
                    exit(EXIT_FAILURE);
                } else if (rc == 0){
                            //child
                    int err = execv(cmd_w_path, commands);
                    if (err = -1){
                        int errsv = errno;
                        printf("Couldn't launch application. Error code %d\n",errsv);
                    }
                    exit(0);
                } else {
                    //Parent path
                    int status;
                    wait(&status);
                    //if (cmd_w_path) {free(cmd_w_path);}
                }
            } else {
                fprintf(stderr, "Couldn't find command %s\n", commands[0]);
            }
        }
    }
}

int readParameters(char *input_line, char *commands[250]){
    
        char *input = NULL;
        int counter = 0;
        while((input = strsep(&input_line," ")) != NULL){
            
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, " ") != 0 && strcmp(input, "\n") != 0){
                commands[counter] = input;

                counter++;
            }
        }
        return counter;
}

int splitPathVar(char *path, char *paths[250]){

        char *input = NULL;
        char *temp_path;
        temp_path = (char*) malloc(1000);
        strcpy(temp_path, path);
        int counter = 0;
        while((input = strsep(&temp_path,":")) != NULL){
            
            input[strcspn(input, "\n")] = 0;
            
            if (strcmp(input, " ") != 0 && strcmp(input, "\n") != 0){
                paths[counter] = input;

                counter++;
            }
        }
        free(temp_path);
        return counter;
}

int getCorrectPath(char *command, int n_paths, char *paths[250], char *cmd_w_path){
    for (int i = 0; i < n_paths; i++){
       char *path;
       path = (char*) malloc(200);
       strcpy(path, paths[i]);
       strcat(path, "/");
       strcat(path, command);

       if (!access(path, X_OK)){
           strcpy(cmd_w_path, path);
           free(path);
           return 1;
       } else {
           int errsv = errno;
           //fprintf(stderr,"Couldn't access path, err no %d\n",errsv);
       }
    }
    return 0;
}
