//Name: Kyra Phillips
//I received help from: Myself, man pages, Holly Radcliffe
//I provided help to: Slightly helped Holly Radcliffe
//Resources used: Class notes and the people who helped me

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h> 
#include <assert.h>
// Function to create handler

void handler (int signum){
  write(1,"K",1);
  switch(signum){
  case 1:
    signal(SIGIO, handler);
    write(1,"SIGIO",5);
    break;
  case 2:
    signal(SIGALRM, handler);
    write(1,"SIGALRM",7);
    break;
  case 3:
    signal(SIGUSR1, handler);
    write(1,"SIGUSR1",7);
    break;
  } 
}
 
//Main function to be a signal handler to receive signal 
int main (int argc, char **argv){
  pid_t pid;
  pid = fork();
  pid_t ppid;
  ppid = getpid();

  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset (&action.sa_mask);
  assert(sigaction(SIGUSR1,&action,NULL) ==0);
  
  if (pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (pid > 0){
    int status;
    int result = waitpid(pid, &status, NULL);
    if (result == -1){
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)){
      WEXITSTATUS(status);
    }
  }
  else {
    assert;
    assert(kill(ppid, SIGIO) == 0);
    assert(kill(ppid, SIGUSR1) == 0);
  }
  return 0;
}
