#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define READ 0
#define WRITE 1

#define TO_KERNEL 3
#define FROM_KERNEL 4

/*
A kernel call is made by putting a request in the pipe from the child to the kernel and then sending the kernel a SIGTRAP
*/




int main (int argc, char** argv)
{

    int pid = getpid();
    int ppid = getppid();

    printf ("writing in pid %d\n", pid);
    //const char *message = "from the process to the kernel \n";
    //write (TO_KERNEL, message, strlen (message));

    //const char *message3 = "Please send the list of processes";
    const char *message3 = "p";
    write (TO_KERNEL, message3, strlen (message3));
    //kill (ppid, SIGTRAP);

    //const char *message2 = "Please send the system time";
    const char *message2 = "t";
    write (TO_KERNEL, message2, strlen (message2));
    kill (ppid, SIGTRAP);

    //printf ("trapping to %d in pid %d\n", ppid, pid);
    //kill (ppid, SIGTRAP);
    //sleep(1);

    printf ("reading in pid %d\n", pid);
    char buf[1024];
    int num_read = read (FROM_KERNEL, buf, 1023);
    buf[num_read] = '\0';
    printf ("process %d read: %s\n", pid, buf);

    sleep(1);

	//for (int i = 0; i < 6;i++)
	//{
	//    printf("%d\n", i);
	//    sleep 1;
	//}



    exit (0);
}
