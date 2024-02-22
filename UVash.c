#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

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

int executeStandardCommand(char *args[], char *redirection, int mode)
{
    int stream;
    pid_t pid;
    stream = open(redirection, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    pid = fork();
    if (pid == 0)
    {
        if (mode > 1)
        {
            dup2(stream, 1);
            dup2(stream, 2);
        }
        execvp(args[0], args);
    }
    else
    {
        wait(NULL);
    }
    return 0;
}

int parseCommand(char *subString)
{
    char *redirections[25];
    char *found;
    int i = 0;
    int mode = 0;
    char *args[100];
    int j = 0;
    char *delim = " \t";
    while ((found = strsep(&subString, ">")) != NULL)
    {
        if (*found != '\0')
        {
            redirections[i++] = found;
            mode++;
        }
    }
    for (int k = i; k < 25; k++)
    {
        redirections[k] = NULL;
    }
    while ((found = strsep(&redirections[0], delim)) != NULL)
    {
        if (*found != '\0')
        {
            args[j++] = found;
        }
    }
    for (int k = j; k < 100; k++)
    {
        args[k] = NULL;
    }
    for (int k = 1; k < mode; k++)
    {
        while ((found = strsep(&redirections[k], delim)) != NULL)
        {
            if (*found != '\0')
            {
                redirections[k] = found;
                break;
            }
        }
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
        executeStandardCommand(args, redirections[1], mode);

    }
    return 0;
}

int parseLine(char *line)
{
    char *found;
    char *delim = "&";
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