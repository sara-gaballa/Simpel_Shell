#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define  MAX_LENGTH 1024
FILE *fptr;
void removing_qoutes(char *string){
    int j = 0;
    char new_string[1024];
    for (int i = 0; string[i] != '\0' && string[i] != '\n' ; ++i) {
        if (string[i] != '\"') {
            new_string[j] = string[i];
            j++;
        }
    }
    new_string[j] = '\0';
    strcpy(string, new_string);
}

void execute_shell_bultin(char **command, char *argument[], int AR_NUM){  ////
    if(!strcmp(*command,"cd")){
        if (argument[0] == NULL) {
//            chdir(getenv(""));
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        }else if (!strcmp(argument[0], "~")) {
            chdir(getenv("HOME"));
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        } else if (!strcmp(argument[0], "..")) {
            chdir("../..");
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        } else if (chdir(argument[0]) == -1) {
            printf("cd: %s: %s\n",argument[0], strerror(errno));
        } else {
            chdir(argument[0]);
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        }
    }else if(!strcmp(*command,"export")){
        char *var,*value;
        var = strtok(argument[0], "=");
        var[strlen(var)] = '\0';
        printf("%d    ", strlen(var));
        value = strtok(NULL, "\n");
        value[strlen(value)] = '\0';
        // printf("%s  %s", var, value);
        removing_qoutes(value);
        if (setenv(var, value, 1) != 0) {
            perror("setenv error");
            exit(EXIT_FAILURE);
        }
        printf("%s=%s\n", var, getenv(var));
    }else if(!strcmp(*command,"echo")){
        char *arg = argument[0]; //'hello $x'
        printf("%s   %d\n",argument[0], strlen(argument[0]));
        removing_qoutes(arg);
        printf("%s   %d\n",arg, strlen(arg));
        for (int i = 0; i < strlen(arg); ++i) {
            if(arg[i] == '$'){
                int j = 0;
                i++;
                char *var;
                while (arg[i] != '\0' && arg[i] != ' '){
                    var[j++] = arg[i++];
                }
                i--;
                var[j] ='\0';
                printf("%s",getenv(var));
            }
            else{
                printf("%c",arg[i]);
            }
        }
        printf("\n");
    }
}
//for no &
//back &
void execute_command(char **args, int ground){
    pid_t pid = fork();
    if (pid == 0) { /* Child process */
        execvp(args[0], args);
        printf("Command not found\n");
        exit(1);
    } else if (pid > 0 && !ground) { /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        fptr = fopen("log.txt","a");
        fprintf(fptr,"%s","child terminated\n");
        fclose(fptr);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Command returned non-zero exit status\n");
        }
    } else { /* Error */
        printf("Failed to fork process\n");
    }
}
void evaluate_expression(char input[], char **command, char *argument[], int *AR_NUM, int *built_in ){
    char *Parsed_Input;
    Parsed_Input = strtok(input, " ");
    *command = Parsed_Input;
    int len = strlen(*command);
    if(command[0][len - 1] == '\n') // removing extra \n
        command[0][len - 1] = '\0';

    if(!strcmp(*command,"cd") || !strcmp(*command,"export") || !strcmp(*command,"echo")) { //check for built-in commands
        *built_in = 1;
        argument[0] = strtok(NULL, "\n");
        //argument[0][strlen(argument[0])] = '\n';
        printf("%d\n", strlen(argument[0])); //taking the single argument of the built-in commands
    }
    else {
        //TO DO export arguments and echo
        int I_A = 0; //index of arguments
        while (Parsed_Input != NULL) {
            Parsed_Input = strtok(NULL, " ");
            if (Parsed_Input != NULL) {
                argument[I_A] = Parsed_Input;
                int len = strlen(argument[I_A]);
                if (argument[I_A][len - 1] == '\n')// removing extra \n
                    argument[I_A][len - 1] = '\0';
                I_A += 1;
                *AR_NUM += 1;
            }
        }
        argument[I_A] = NULL;
    }
}
void setup_environment(){
    char path[1024];
    chdir(getcwd(path,sizeof (path)));
}
void proc_exit()//
{
    int wstat;
    pid_t pid;
    while (1) {
       // printf("hehehe");
        pid = wait3(&wstat, WNOHANG, NULL);
        fptr = fopen("log.txt","a");
        fprintf(fptr,"Return code: %d\n", wstat);
        if (pid == 0 || pid == -1)
            return;
        fclose(fptr);
    }
}
int main(int argc, char *argv[])
{

//    if(fptr == NULL)
//    {
//        printf("Error!");
//        exit(1);
//    }
    signal (SIGCHLD, proc_exit);
    setup_environment();
    while (1){
       // printf("ddw");
        char input[MAX_LENGTH];
        char *command;
        char *argument[10];
        int Arr_NUM = 0;
        int B_IN = 0;
        fgets(input, sizeof(input), stdin);
        if(!strcmp(input, "\n")){ //if the user enters an empty line
            continue;
        }
        //taking commands and arguments from the input
        evaluate_expression(input, &command, argument, &Arr_NUM, &B_IN);
        printf( "%d %d\n", Arr_NUM, B_IN);
        //printf( "%s\n", argument[0]);

        if(!strcmp(command,"exit")){
            exit(0);
        }
        else {
            if (B_IN == 0) {
                int ground = 0; //no & forground
                char *args[Arr_NUM + 2];
                args[0] = command;
                args[Arr_NUM + 1] = NULL;
                for (int i = 0; i < Arr_NUM; ++i) {
                    if(argument[i][0] == '$'){
                        char*  tmp = strsep(&argument[0], "$");
                        char* var = strsep(&argument[0], " ");
                        printf("%s\n", getenv(var));

                        char* parse = strtok(getenv(var), " ");
                        while (parse != NULL){
                            args[++i] = parse;
                            printf("%s\n",args[i]);
                            parse = strtok(NULL, " ");
                        }
                        args[i + 1] = NULL;
                        break;
                    }
                    else{
                    args[ i + 1 ] = argument[i];
                    if(argument[i][0] == '&'){
                        ground = 1;
                        break;
                    }
                    }
                }
                execute_command(&args, ground); //args must terminate with null
            } else {
                execute_shell_bultin(&command, argument , Arr_NUM );
            }
        }
    }

}

