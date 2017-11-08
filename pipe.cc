#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#define WRITEFD(fd,a) { const char *foo = a; \
    assert(write(fd, foo, strlen (foo)) != -1); }

struct pipes {
    int child2parent[2];
    int parent2child[2];
} pipes;

#define READ 0
#define WRITE 1

void sigchild (int signum) {
    assert(signum == SIGCHLD);
    exit(0);
}

void sigtrap (int signum) {
    assert(signum == SIGTRAP);

    // for more than one process, poll as we don't know which
    // process sent the signal. here, there's only one process
    // so no need to poll
    char buffer[1024];
    int len = read(pipes.child2parent[READ], buffer, sizeof (buffer));
    assert(len != -1);
    buffer[len] = 0;
    WRITEFD(STDOUT_FILENO, buffer);
    WRITEFD(pipes.parent2child[WRITE], "Hello child\n");
}

int main (int argc, char** argv) {
    struct sigaction trap_action;
    trap_action.sa_handler = sigtrap;
    trap_action.sa_flags = 0;
    assert(sigemptyset (&trap_action.sa_mask) == 0);
    assert(sigaction (SIGTRAP, &trap_action, NULL) == 0);

    struct sigaction child_action;
    child_action.sa_handler = sigchild;
    child_action.sa_flags = 0;
    assert(sigemptyset(&child_action.sa_mask) == 0);
    assert(sigaction (SIGCHLD, &child_action, NULL) == 0);

    assert(pipe(pipes.child2parent) == 0);
    assert(pipe(pipes.parent2child) == 0);

    // make the read end of the parent non-blocking
    int fl = fcntl(pipes.child2parent[READ], F_GETFL);
    assert(fl != -1);
    assert(fcntl(pipes.child2parent[READ], F_SETFL, fl | O_NONBLOCK) == 0);

    if (fork() == 0) {
        // close the ends we should't use
        assert(close(pipes.child2parent[READ]) == 0);
        assert(close(pipes.parent2child[WRITE]) == 0);

        WRITEFD(pipes.child2parent[WRITE], "Hello parent\n");
        assert(kill(getppid(), SIGTRAP) != -1);

        char buffer[1024];
        int len = read(pipes.parent2child[READ], buffer, sizeof (buffer));
        assert(len != -1);
        buffer[len] = 0;
        WRITEFD(STDOUT_FILENO, buffer);
        exit (0);
    } else {
        // close the ends we should't use
        assert(close(pipes.child2parent[WRITE]) == 0);
        assert(close(pipes.parent2child[READ]) == 0);

        for (;;) {
           pause();
        }
    }
}
