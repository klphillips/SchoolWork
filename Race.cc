//Name: Kyra Phillips
//I received help from: Holly Radcliffe
//I provided help to: Tatiana Smith
//Resources used: Man pages, and this website: https://computing.llnl.gov/tutorials/pthreads/#Mutexes

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*
** Compile and run this program, and make sure you get the 'aargh' error
** message. Fix it using a pthread mutex. The one command-line argument is
** the number of times to loop. Here are some suggested initial values, but
** you might have to tune them to your machine.
** Debian 8: 100000000
** Gouda: 10000000
** OS X: 100000
** You will need to compile your program with the "-lpthread" option at
** the end of gcc/g++ command.
*/

#define SYSASSERT(a) if (!(a)) { \
    fprintf(stderr, "assertion %s failed: file %s, line %d, error %s\n", \
        #a, __FILE__, __LINE__, strerror(errno)); exit(1); }
#define NUM_THREADS 2
pthread_mutex_t mutex;
int i;

void *foo (void *bar) {
    pthread_t *me = new pthread_t (pthread_self());
    SYSASSERT(printf ("in a foo thread, ID %ld\n", *me) != 0);
    SYSASSERT(pthread_mutex_lock(&mutex) == 0);
    for (i = 0; i < * ((int *) bar); i++) {
        int tmp = i;

        if (tmp != i) {
            SYSASSERT(printf("aargh: %d != %d\n", tmp, i) != 0);
        }
    }
    SYSASSERT(pthread_mutex_unlock(&mutex) == 0);
    pthread_exit (me);
}

int main (int argc, char **argv) {
    int iterations = strtol (argv[1], NULL, 10);
    pthread_t threads[NUM_THREADS];
    SYSASSERT(pthread_mutex_init(&mutex,NULL) ==0);

    for (int i = 0; i < NUM_THREADS; i++) {
        SYSASSERT(pthread_create(&threads[i], NULL, foo,
            (void *) &iterations) == 0);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        void *status;

        SYSASSERT(pthread_join (threads[i], &status) == 0);
        SYSASSERT(printf ("joined a foo thread, number %ld\n",
            * ((pthread_t *) status)) != 0);
    }
    SYSASSERT(pthread_mutex_destroy(&mutex) == 0);
    return (0);
}
