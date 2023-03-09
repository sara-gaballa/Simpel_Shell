#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define  MAX_LENGTH 1024
FILE *fptr; //public pointer to the text file
void Handler(){
    int wstat;
    pid_t pid;
    while (1) {
        pid = wait3(&wstat, WNOHANG, NULL);
        fptr = fopen("log.txt","a");
        fprintf(fptr,"ZOMBIES TERMINATED , Return code: %d\n", wstat);
        if (pid == 0 || pid == -1)
            return;
        fclose(fptr);
    }
}
void removingQoutes(char *string){ //removing qoutes from the string
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
void executeShellBultin(char **command, char *argument[]){//execution of build in functions
    if(!strcmp(*command,"cd")){//for cd command in opening directories
        if (argument[0] == NULL) {//returning to the home path if null is entered
            chdir(getenv("HOME"));
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        }else if (!strcmp(argument[0], "~")) {//returning to the home path
            chdir(getenv("HOME"));
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        } else if (!strcmp(argument[0], "..")) {//retuning two places backwards in directory location
            chdir("../..");
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        } else if (chdir(argument[0]) == -1) {
            printf("cd: %s: %s\n",argument[0], strerror(errno));
        } else {//opening the path entered by the user
            chdir(argument[0]);
            char path[1024];
            printf("The current path: %s\n",getcwd(path,sizeof (path)));
        }
    }else if(!strcmp(*command,"export")){ //for export and evaluating the expression
        char *var,*value;//pointers for saving variable and value
        var = strtok(argument[0], "=");//separation at "="
        var[strlen(var)] = '\0';//adding null char to the end of the string
        value = strtok(NULL, "\n");
        value[strlen(value)] = '\0';
        removingQoutes(value);//removing the qoutes if available
        if (setenv(var, value, 1) != 0) {//saving the value to the variable using setenv()
            perror("setenv error");
            exit(EXIT_FAILURE);
        }
    }else if(!strcmp(*command,"echo")){//for echo command
        char *arg = argument[0]; //'hello $x'
        removingQoutes(arg);
        for (int i = 0; i < strlen(arg); ++i) {
            if(arg[i] == '$'){
                int j = 0;
                i++;
                char *var;
                while (arg[i] != '\0' && arg[i] != ' '){//extracting the variable from the entered string
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
void execute_command(char **args, int ground){//executing commands
    pid_t pid = fork();//forking the child processes
    if (pid == 0) { /* Child process */
        execvp(args[0], args);
        printf("Command not found\n");
        exit(1);
    } else if (pid > 0 && !ground) { /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        fptr = fopen("log.txt","a");
        fprintf(fptr,"%s%d%s","Child ",pid," is terminated\n");
        fclose(fptr);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Command returned non-zero exit status\n");
        }
    } else if(pid == -1){ /* Error */
        perror("fork");
        exit(1);
    }
}
void Parse_Input(char input[], char **command, char *argument[], int *AR_NUM, int *built_in ){//parsing input entered by the user
    char *Parsed_Input;
    Parsed_Input = strtok(input, " ");
    *command = Parsed_Input;
    int len = strlen(*command);
    if(command[0][len - 1] == '\n') // removing extra \n
        command[0][len - 1] = '\0';
    if(!strcmp(*command,"cd") || !strcmp(*command,"export") || !strcmp(*command,"echo")) { //check for built-in commands
        *built_in = 1;
        argument[0] = strtok(NULL, "\n");
    }
    else {//extracting the arguments
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
void setup_environment(){//setting the working directory to the current directory
    char path[1024];
    chdir(getcwd(path,sizeof (path)));
}
int main()
{
    fptr = fopen("log.txt", "w");//erase the file from any previous content
    fclose(fptr);
    signal(SIGCHLD, Handler);//setting signal handler
    setup_environment();
    while (1){
        char input[MAX_LENGTH];
        char *command;
        char *argument[10];
        int Arr_NUM = 0;//number of arguments
        int B_IN = 0;//boolean to check if build in function or not
        printf("SHELL-> ");
        fgets(input, sizeof(input), stdin);
        if(!strcmp(input, "\n")){ //if the user enters an empty line
            continue;
        }
        Parse_Input(input, &command, argument, &Arr_NUM, &B_IN);//taking commands and arguments from the input
        if(!strcmp(command,"exit")){ //exit command to terminate the shell
            exit(0);
        }
        else {
            if (B_IN == 0) {
                int ground = 0; //no & forground
                char *args[Arr_NUM + 2];
                args[0] = command; //saving the command in the first element of the array
                args[Arr_NUM + 1] = NULL;//terminating the args with null
                for (int i = 0; i < Arr_NUM; ++i) {
                    if(argument[i][0] == '$'){
                        char*  tmp = strsep(&argument[0], "$");
                        char* var = strsep(&argument[0], " ");
                        char* parse = strtok(getenv(var), " ");
                        while (parse != NULL){
                            args[++i] = parse;
                            parse = strtok(NULL, " ");
                        }
                        args[i + 1] = NULL;
                        break;
                    }
                    else{
                        if(argument[i][0] == '&'){
                            ground = 1;
                            break;
                        }
                        args[ i + 1 ] = argument[i];
                    }
                }
                execute_command(&args, ground); //args must terminate with null
            } else {
                executeShellBultin(&command, argument);//if the commad is from build in functions
            }
        }
    }
}

