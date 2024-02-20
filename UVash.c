#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int errorMessage()
{
    char error_message[30] = "An error has occurred\n";
    fprintf(stderr, "%s", error_message);
    return 1;
}

int executeChdir(char *args[])
{
    if (args[1] == NULL || args[2] != NULL)
    {
        errorMessage();
        return 1;
    }
    if (!chdir(args[1]))
    {
        errorMessage();
    }
    return 0;
}

int executeStandardCommand(char *args[])
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        execvp(args[0], args);
    }
    else
    {
        wait(NULL);
    }
    return 0;
}

int parseCommand(char *subString) {
    char *args[100];
    int j = 0;
    char *found;
    char *delim = " \t";
    while ((found = strsep(&subString, delim)) != NULL)
    {
        if (*found != '\0')
        {
            args[j++] = found;
        }
    }
    while (j < 100)
    {
        args[j++] = NULL;
    }
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        executeChdir(args);
    }
    else
    {
        executeStandardCommand(args);
    }
    return 0;
}


int parseLine(char *line)
{
    char *found;
    char *delim = " &";
    while ((found = strsep(&line, delim)) != NULL)
    {
        if (*found != '\0')
        {
            parseCommand(found);
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
        printf("UVash> ");
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
        line[strlen(line) - 1] = '\0';
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