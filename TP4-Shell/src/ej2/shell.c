#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 100

void interpret_escape_sequences(char *str) {
    char *src = str;
    char *dst = str;
    while (*src) {
        if (*src == '\\' && *(src + 1)) {
            switch (*(src + 1)) {
                case 'n': *dst++ = '\n'; src += 2; break;
                case 't': *dst++ = '\t'; src += 2; break;
                default: *dst++ = *src++; break;
            }
        } else { *dst++ = *src++; }
    }
    *dst = '\0';
}
int parse_arguments(char *command, char **args) {
    int arg_count = 0;
    char *p = command;
    char quote_char = '\0';

    while (*p) {
        while (isspace(*p)) p++;
        if (*p == '\0') break;

        char *start = p;
        if (*p == '"' || *p == '\'') {
            quote_char = *p++;
            start = p;
            while (*p && *p != quote_char) p++;
            if (*p) *p++ = '\0';
            else {
                fprintf(stderr, "Error: comillas sin cerrar\n");
                return -1;
            }
        } else {
            while (*p && !isspace(*p)) p++;
            if (*p) *p++ = '\0';
        }
        args[arg_count++] = start;
    }

    args[arg_count] = NULL;
    return arg_count;
}

int split_commands(char *input, char **commands) {
    int count = 0;
    char *p = input;
    char *start = p;
    int in_quotes = 0;
    char quote_char = '\0';

    while (*p) {
        if ((*p == '"' || *p == '\'') && (in_quotes == 0 || *p == quote_char)) {
            if (in_quotes == 0) {
                in_quotes = 1;
                quote_char = *p;
            } else {
                in_quotes = 0;
            }
        } else if (*p == '|' && !in_quotes) {
            *p = '\0';
            commands[count++] = start;
            start = p + 1;
        }
        p++;
    }

    commands[count++] = start;
    return count;
}

int main() {
    char command[4096];
    pid_t pids[MAX_COMMANDS];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        if (isatty(STDIN_FILENO)) printf("Shell> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            if (isatty(STDIN_FILENO)) printf("\nEOF\n");
            break;
        }
        command[strcspn(command, "\n")] = '\0';

        command_count = split_commands(command, commands);

        int pipes[command_count - 1][2];
        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipes[i]) == -1) { perror("pipe"); exit(1); }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid == -1) { perror("fork"); exit(1); }
            pids[i] = pid;
            if (pid == 0) {
                if (i > 0) {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                if (i < command_count -1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                for (int j=0;j<command_count-1;j++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                char *args[MAX_ARGS];
                int n = parse_arguments(commands[i], args);
                if (n <= 0 || args[0]==NULL) { fprintf(stderr, "Error comando inválido\n"); exit(1); }

                if (strcmp(args[0],"echo")==0 && args[1] && strcmp(args[1],"-e")==0) {
                    interpret_escape_sequences(args[2]);
                    for (int j=1;args[j];j++) args[j]=args[j+1];
                }

                execvp(args[0],args);
                perror("execvp");
                exit(1);
            }
        }

        for (int i=0;i<command_count-1;i++){
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i=0;i<command_count;i++){
            waitpid(pids[i],NULL,0);
        }
    }
    return 0;
}

