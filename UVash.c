#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

typedef struct
{
    char *name;
    char *args[100];
    char *redirections[25];
    int numberOfRedirections;
} command;

int errorMessage()
{
    char error_message[30] = "An error has occurred\n";
    fprintf(stderr, "%s", error_message);
    return 1;
}

int printPrompt()
{
    char cwd[100];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        errorMessage();
        return 1;
    }
    fprintf(stdout, "\033[0;32mUVash\033[0m:\033[0;34m%s\033[0m:$ ", cwd);
    return 0;
}

int executeChdir(char *args[])
{
    if (args[1] == NULL || args[2] != NULL)
    {
        errorMessage();
        return 1;
    }
    if (chdir(args[1]))
    {
        errorMessage();
        return 1;
    }
    return 0;
}

int executeStandardCommand(char *args[], char *redirections[], int mode)
{
    int stream;
    pid_t pid;
    char *found;
    char *fileName;
    int onlyOne = 0;
    while ((found = strsep(&redirections[1], " \t")) != NULL)
    {
        if (*found != '\0')
        {
            if (onlyOne == 1 && mode > 1)
            {
                errorMessage();
                return 1;
            }
            onlyOne = 1;
            fileName = found;
        }
    }
    stream = open(fileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    pid = fork();

    if (pid == 0)
    {
        if (mode > 1)
        {

            if (onlyOne == 0)
            {
                errorMessage();
                return 1;
            }
            dup2(stream, 1);
            dup2(stream, 2);
        }
        if (execvp(args[0], args) == -1)
        {
            errorMessage();
            return 1;
        }
    }
    else
    {
        wait(NULL);
    }
    return 0;
}

int split(char *string, char *delim, char *array[], int arraySize)
{
    char *found;
    int i = 0;
    int size = 0;
    while ((found = strsep(&string, delim)) != NULL)
    {
        if (*found != '\0')
        {
            array[i++] = found;
        }
        size++;
    }
    for (int k = i; k < arraySize; k++)
    {
        array[k] = NULL;
    }
    return size;
}

int parseCommand(char *subString, command *command)
{
    char *redirections[25];
    char *found;
    int i = 0;
    int mode = 0;
    char *args[100];
    int j = 0;
    mode = split(subString, ">", &redirections, 25);
    split(redirections[0], " \t", &args, 100);
    command->name = args[0];
    for (int k = 0; k < 100; k++)
    {
        command->args[k] = args[k];
    }
    for (int k = 0; k < 25; k++)
    {
        command->redirections[k] = redirections[k];
    }
    command->numberOfRedirections = mode;
    return 0;
}

int parseLine(char *line)
{
    line = strsep(&line, "\n");
    command commands[100];
    char *found;
    char *delim = "&";
    int i = 0;
    char *redirections[25];
    int mode = 0;
    char *args[100];
    while ((found = strsep(&line, delim)) != NULL)
    {
        if (*found != '\0')
        {
            parseCommand(found, &commands[i++]);
            args[0] = commands[i - 1].name;
            for (int k = 0; k < 100; k++)
            {
                args[k] = commands[i - 1].args[k];
            }
            for (int k = 0; k < 25; k++)
            {
                redirections[k] = commands[i - 1].redirections[k];
            }
            mode = commands[i - 1].numberOfRedirections;
            if (strcmp(args[0], "exit") == 0)
            {
                if (args[1] != NULL)
                {
                    errorMessage();
                    return 1;
                }
                exit(0);
            }
            else if (strcmp(args[0], "cd") == 0)
            {
                executeChdir(args);
            }
            else
            {

                executeStandardCommand(args, redirections, mode);
            }
        }
    }
    return 0;
}

int interactiveMode()
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    while (1)
    {
        printPrompt();
        nread = getline(&line, &len, stdin);
        if (nread == -1)
        {
            if (feof(stdin))
            {
                exit(0);
            }
            errorMessage();
            exit(1);
        }
        line[strlen(line) - 1] = '\0';
        parseLine(line);
    }
    free(line);
    exit(0);
    return 0;
}

int batchMode(char *filePath)
{
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    stream = fopen(filePath, "r");
    if (stream == NULL)
    {
        errorMessage();
        exit(1);
    }
    while ((nread = getline(&line, &len, stream)) != -1)
    {
        parseLine(line);
    }
    if (!feof(stream))
    {
        errorMessage();
        exit(1);
    }
    free(line);
    fclose(stream);
    exit(0);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        // Interactive mode
        interactiveMode();
    }
    else if (argc == 2)
    {
        // Batch mode
        batchMode(argv[1]);
    }
    else
    {
        // Error
        errorMessage();
        exit(1);
    }
    return 0;
}