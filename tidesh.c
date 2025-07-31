#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>

// Constants
#define TIDESH_RL_BUFSIZE 1024
#define TIDESH_TOK_BUFSIZE 64
#define TIDESH_TOK_DELIM " \t\r\n\a"

// Built-in function declarations
int tidesh_cd(char **args);
int tidesh_help(char **args);
int tidesh_exit(char **args);
int tidesh_launch(char **args);

// Signal handler
void handle_sigint(int sig) {
  write(STDOUT_FILENO, "\nInterrupted.\n> ", 15);
}

// Read line from stdin manually
char *tidesh_read_line(void) {
  int bufsize = TIDESH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "tidesh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position++] = c;
    }

    if (position >= bufsize - 1) {
      bufsize += TIDESH_RL_BUFSIZE;
      char *new_buffer = realloc(buffer, bufsize);
      if (!new_buffer) {
        free(buffer);
        fprintf(stderr, "tidesh: allocation error\n");
        exit(EXIT_FAILURE);
      }
      buffer = new_buffer;
    }
  }
}

// Tokenize input
char **tidesh_split_line(char *line) {
  int bufsize = TIDESH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "tidesh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TIDESH_TOK_DELIM);
  while (token != NULL) {
    tokens[position++] = token;

    if (position >= bufsize) {
      bufsize += TIDESH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "tidesh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TIDESH_TOK_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}

// Built-in command list
char *builtin_str[] = { "cd", "help", "exit" };
int (*builtin_func[]) (char **) = { &tidesh_cd, &tidesh_help, &tidesh_exit };

int tidesh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Execute built-in or external
int tidesh_execute(char **args) {
  if (args[0] == NULL) {
    return 1; // ignore empty input
  }

  for (int i = 0; i < tidesh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return tidesh_launch(args);
}

// Fork + execvp
int tidesh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("tidesh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("tidesh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// Built-in commands
int tidesh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "tidesh: expected argument to \"cd\"\n");
  } else if (chdir(args[1]) != 0) {
    perror("tidesh");
  }
  return 1;
}

int tidesh_help(char **args) {
  printf("tidesh - a very soggy shell.\nBuilt-in commands:\n");
  for (int i = 0; i < tidesh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  return 1;
}

int tidesh_exit(char **args) {
  return 0;
}

// Main loop
void tidesh_loop(void) {
  char *line;
  char **args;
  int status;
  char cwd[PATH_MAX];

  do {
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("%s > ", cwd);
    } else {
      printf("> ");
    }

    line = tidesh_read_line();
    args = tidesh_split_line(line);
    status = tidesh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  signal(SIGINT, handle_sigint); // Ctrl+C handler
  tidesh_loop();
  return EXIT_SUCCESS;
}
