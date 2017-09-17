//Name: Kyra Phillips
//I received help from: Myself, man pages, Holly Radcliffe, Dr Beaty
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
  switch(signum){
  case SIGIO:
    signal(SIGIO, handler);
    write(1,"SIGIO\n",6);
    break;
  case SIGALRM:
    signal(SIGALRM, handler);
    write(1,"SIGALRM\n",8);
    break;
  case SIGUSR1:
    signal(SIGUSR1, handler);
    write(1,"SIGUSR1\n",8);
    break;
  } 
}
 
//Main function to be a signal handler to receive signal 
int main (int argc, char **argv){

  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset (&action.sa_mask);
  assert(sigaction(SIGUSR1,&action,NULL) ==0);
  assert(sigaction(SIGIO,&action,NULL)==0);
  assert(sigaction(SIGALRM,&action,NULL)==0);

  pid_t pid;
  pid = fork();
  pid_t ppid;
  ppid = getpid();

  
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
    assert(kill(ppid,SIGALRM) ==0);
    assert(kill(ppid, SIGIO) == 0);
    assert(kill(ppid, SIGUSR1) == 0);
  }
  return 0;
}
