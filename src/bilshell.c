#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int i;
int is_piped; // 0 if normal command 1 if piped command others if not valid
int num_of_chars; // N value
char *input_line = NULL; // input for interractive mode
size_t input_size = 0; // input size for interactive mode
char input_txt_line[100]; // 1 line of input for batch mode
int input_size_txt = 100; // its size
char *temp_str; // variable for fix_input function
char *fixed_input_left[1000000]; // if normal command is runned only this will given input
char *fixed_input_right[1000000]; // if piped command is runned second commands is this.

/*
return == 0 -> possible normal command
return == 1 -> possible pipe command
return > 1 -> not valid command
organize the given input for execvp() call
*/
int fix_input(char *input)
{
    int mode_type = 0;
    int temp_i;
    temp_str = strtok(input, "\n ");
    for(i = 0; temp_str != NULL; i++)
    {
        if(strcmp(temp_str, "|") == 0)
        {
            temp_i = i + 1;
            i = -1;
            mode_type = mode_type + 1;
        }
        else if(mode_type == 0)
        {
            fixed_input_left[i] = temp_str;
        }
        else
        {
            fixed_input_right[i] = temp_str;
        }
        temp_str = strtok(NULL,"\n ");

    }
    if(mode_type == 0)
        fixed_input_left[i] = NULL;
    else
    {
        fixed_input_left[temp_i] = NULL;
        fixed_input_right[i] = NULL;
    }
    return mode_type;

}

/*
This function basically executes the given command.
*/
void execute_command()
{
    /*
    Without pipe symbol normal execution with 1 child process
    */
    if(is_piped == 0)
    {
        int pid = fork();
        if (pid == 0)
        {
            if(execvp(fixed_input_left[0], fixed_input_left) == -1)
            {
                printf("No such command or file found");
                exit(errno);
            }
        }
        else
        {
            wait(NULL);
        }
    }
    /*
    If pipe symbol provided piped execution with 2 child and 2 pipe
    */
    else if(is_piped == 1)
    {
        int pipe1[2];
        int pipe2[2];
        pipe(pipe1);
        pipe(pipe2);
        int pid1 = fork();
        if(pid1 == 0) // child 1
        {
            dup2(pipe1[1],1);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            if(execvp(fixed_input_left[0], fixed_input_left) == -1)
            {
                printf("No such command or file found");
                exit(errno);
            }
        }
        else
        {
            int pid2 = fork();
            if(pid2 == 0) // child 2
            {
                dup2(pipe2[0],0);
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[0]);
                close(pipe2[1]);
                if(execvp(fixed_input_right[0], fixed_input_right) == -1)
                {
                    printf("No such command or file found");
                    exit(errno);
                }

            }
            else // parent
            {
                close(pipe1[1]);
                close(pipe2[0]);
                char *read_buff;
                read_buff = (char *)malloc(sizeof(int) * num_of_chars);
                int char_count = 0;
                int read_write_call_count = 0;
                /*
                Main process read from pipe1 and writes to pipe2 until pipe1 is gets empty
                */
                while(read(pipe1[0], read_buff, num_of_chars))
                {
                    write(pipe2[1], read_buff, num_of_chars);
                    char_count = char_count + num_of_chars;
                    read_write_call_count = read_write_call_count + 1;
                }
                close(pipe1[0]);
                close(pipe2[1]);
                wait(NULL);
                wait(NULL);
                printf("\ncharacter-count(byte-count): %d", char_count);
                printf("\nread-and-write-call-count: %d", read_write_call_count);
            }
        }
    }
    /*
    If user try to enter more then 1 "|" symbol error message will provided.
    */
    else
    {
        printf("\nError: More then 1 \"|\" symbol is not supported");
    }

}


int main(int argc, char ** argv)
{
    /* Interactive Mode */
    if(argc == 2)
    {
        num_of_chars = atoi(argv[1]);
        /*
        Checking the value of N
        */
        if(num_of_chars < 1 || num_of_chars > 4096)
        {
            printf("Number of characters to read must be between 1 and 4096\n");
            return 0;
        }
        printf("Welcome to Interactive mode of the Bilshell! write exit to exit. \n");
        /*
        Until user writes exit bilshell will continue to run at interactive mode
        */
        while(1)
        {
            printf("\nbilshell-$:"); //prompt
            getline(&input_line, &input_size, stdin); // take input
            /*Check if input is empty, if it is empty warning message shown.
            and loop start again
            */
            if(strcmp(input_line,"\n") == 0)
            {
                printf("\nType a Command!");
                continue;
            }
            is_piped = fix_input(input_line); //call for fix_input to organize the command
            /*
            If user typed exit program exists.
            */
            if (strcmp(fixed_input_left[0], "exit") == 0)
            {
                printf("Thanks for using bilshell, have a great day.\n");
                return 0;
            }
            //execute the organized command
            execute_command();
        }
    }
    /*Batch Mode*/
    else if(argc == 3)
    {
        FILE *file;
        num_of_chars = atoi(argv[1]);
        /*
        Checking the value of N
        */
        if(num_of_chars < 1 || num_of_chars > 4096)
        {
            printf("Number of characters to read must be between 1 and 4096\n");
            return 0;
        }
        file = fopen(argv[2], "r");
        /*
        Check if the file exist or not
        */
        if(file == NULL)
        {
            printf("No File exist with the given name.\n");
            return 0;
        }
        printf("Welcome to Batch mode of the Bilshell! Commands will execute 1 by 1\n");
        /*
        Execute commands 1 by 1 until no commands left
        */
        while(fgets(input_txt_line, input_size_txt, file) != NULL)
        {
            is_piped = fix_input(input_txt_line);
            execute_command();
        }
        printf("\nThanks for using bilshell, have a great day. \n");
        return 0;
    }
    /*Wrong execution of bilshell error message given to the user*/
    else
    {
        printf("\nWrong execution.Correct forms are: Bilshell N (or) Bilshell N file.txt ");
        printf("\nFor example if your executable name is Bilshell.");
        printf("\nFor Interactive mode: ./Bilshell 10");
        printf("\nFor batch mode: ./Bilshell 20 test.txt");
        printf("\nN must be between 1 and 4096");
        return 0;
    }
}
