#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];
  int fd[2];
  int status; 
  int wrbytes;
  int result;
  int pid;
  int passwd_result;

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }

  if ((pipe(fd)) == -1 ) {
    perror("pipe");
    exit(1);
  }

  result = fork();
  if (result < 0) {
    perror("fork");
    exit(1);
  } else if (result > 0) {
    if (close(fd[0]) == -1) { // close pipe read
      perror("close");
    }
    if ((wrbytes = write(fd[1], user_id, MAX_PASSWORD)) == -1) {
      perror("write");
      exit(1);
    }
    if ((wrbytes = write(fd[1], password, MAX_PASSWORD)) == -1) {
      perror("write");
      exit(1);
    }

    if (close(fd[1]) == -1) {
      perror("close");
    }
    
    if ((pid = wait(&status)) == -1) {
      perror("wait");
      exit(1);
    }
    if (WIFEXITED(status)) {
      passwd_result = WEXITSTATUS(status);
      if (passwd_result == 0) {
        printf("%s", SUCCESS);
      } else if (passwd_result == 2) {
        printf("%s", INVALID);
      } else if (passwd_result == 3) {
        printf("%s", NO_USER);
      }
    }
  } else {
    if (dup2(fd[0], fileno(stdin)) == -1) {
      perror("dup2");
      exit(1);
    }
    if (close(fd[0]) == -1) {
      perror("close");
    }
    if (close(fd[1]) == -1) {
      perror("close");
    }
    
    execl("./validate", "validate", NULL);
    perror("execl");
  }
  return 0;
}
