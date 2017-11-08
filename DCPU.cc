This program sets up pipes for child processes so that the parrent "CPU.cc" can send and recieve messages to/from
	"child.cc". This project is built off of "CPU.cc" which put processes into a PCB, forked and execled them.
	Then if the processes were not done they were round robined. 
*/
#include <iostream>
#include <list>
#include <iterator>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>


#define READ_END 0
#define WRITE_END 1

// you don't really know at this point how many childern you'll have, so
// don't hard code it here.
//#define NUM_CHILDREN 5

// the way you use this in your PCB, this needs to be 2. 2 pipes for 2-way
// communication between the kernel and a child
#define NUM_PIPES 2

#define WRITE(a) { const char *foo = a; write (1, foo, strlen (foo)); }

// this isn't the right place to create any of the pipes.
//int pipes[NUM_PIPES][2];
int child_count = 0;

#define NUM_SECONDS 20

// make sure the asserts work
#undef NDEBUG
#include <assert.h>

#define EBUG
#ifdef EBUG
#   define dmess(a) cout << "in " << __FILE__ << \
    " at " << __LINE__ << " " << a << endl;

#   define dprint(a) cout << "in " << __FILE__ << \
    " at " << __LINE__ << " " << (#a) << " = " << a << endl;

#   define dprintt(a,b) cout << "in " << __FILE__ << \
    " at " << __LINE__ << " " << a << " " << (#b) << " = " \
    << b << endl
#else
#   define dprint(a)
#endif /* EBUG */

using namespace std;

enum STATE { NEW, RUNNING, WAITING, READY, TERMINATED };

/*
** a signal handler for those signals delivered to this process, but
** not already handled.
*/
void grab (int signum) { dprint (signum); }

// c++decl> declare ISV as array 32 of pointer to function (int) returning
// void
void (*ISV[32])(int) = {
/*        00    01    02    03    04    05    06    07    08    09 */
/*  0 */ grab, grab, grab, grab, grab, grab, grab, grab, grab, grab,
/* 10 */ grab, grab, grab, grab, grab, grab, grab, grab, grab, grab,
/* 20 */ grab, grab, grab, grab, grab, grab, grab, grab, grab, grab,
/* 30 */ grab, grab
};

struct PCB
{
    STATE state;
    const char *name;   // name of the executable
    int pid;            // process id from fork();
    int ppid;           // parent process id
    int interrupts;     // number of times interrupted
    int switches;       // may be < interrupts
    int started;        // the time this process started
    // best to differentiate the two different pipes
    int p2k[2];		    // pipe kernal to process
    int k2p[2];		    // pipe process to kernal
};

/*
** an overloaded output operator that prints a PCB
*/
ostream& operator << (ostream &os, struct PCB *pcb)
{
    os << "state:        " << pcb->state << endl;
    os << "name:         " << pcb->name << endl;
    os << "pid:          " << pcb->pid << endl;
    os << "ppid:         " << pcb->ppid << endl;
    os << "interrupts:   " << pcb->interrupts << endl;
    os << "switches:     " << pcb->switches << endl;
    os << "started:      " << pcb->started << endl;
    return (os);
}

/*
** an overloaded output operator that prints a list of PCBs
*/
ostream& operator << (ostream &os, list<PCB *> which)
{
    list<PCB *>::iterator PCB_iter;
    for (PCB_iter = which.begin(); PCB_iter != which.end(); PCB_iter++)
    {
        os << (*PCB_iter);
    }
    return (os);
}

PCB *running;
PCB *idle;

// http://www.cplusplus.com/reference/list/list/
list<PCB *> new_list;
list<PCB *> processes;

int sys_time;

/*
**  send signal to process pid every interval for number of times.
*/
void send_signals (int signal, int pid, int interval, int number)
{
    dprintt ("at beginning of send_signals", getpid ());

    for (int i = 1; i <= number; i++)
    {
        sleep (interval);

        dprintt ("sending", signal);
        dprintt ("to", pid);

        if (kill (pid, signal) == -1)
        {
            perror ("kill");
            return;
        }
    }
    dmess ("at end of send_signals");
}

struct sigaction *create_handler (int signum, void (*handler)(int))
{
    struct sigaction *action = new (struct sigaction);

    action->sa_handler = handler;
/*
**  SA_NOCLDSTOP
**  If  signum  is  SIGCHLD, do not receive notification when
**  child processes stop (i.e., when child processes  receive
**  one of SIGSTOP, SIGTSTP, SIGTTIN or SIGTTOU).
*/
    if (signum == SIGCHLD)
    {
        action->sa_flags = SA_NOCLDSTOP;
    }
    else
    {
        action->sa_flags = 0;
    }

    sigemptyset (&(action->sa_mask));

    assert (sigaction (signum, action, NULL) == 0);
    return (action);
}

/*
** Async-safe integer to a string. i is assumed to be positive. The number
** of characters converted is returned; -1 will be returned if bufsize is
** less than one or if the string isn't long enough to hold the entire
** number. Numbers are right justified. The base must be between 2 and 16;
** otherwise the string is filled with spaces and -1 is returned.
*/

int eye2eh (int i, char *buf, int bufsize, int base)
{
    if (bufsize < 1) return (-1);
    buf[bufsize-1] = '\0';
    if (bufsize == 1) return (0);
    if (base < 2 || base > 16)
    {
        for (int j = bufsize-2; j >= 0; j--)
        {
            buf[j] = ' ';
        }
        return (-1);
    }

    int count = 0;
    const char *digits = "0123456789ABCDEF";
    for (int j = bufsize-2; j >= 0; j--)
    {
        if (i == 0)
        {
            buf[j] = ' ';
        }
        else
        {
            buf[j] = digits[i%base];
            i = i/base;
            count++;
        }
    }
    if (i != 0) return (-1);
    return (count);
}

PCB* choose_process ()
{
/*
Add kernel calls to your VirtuOS project.
Do this by creating a pair of pipes to every child process (in each PCB). A kernel call is made by putting a request in the pipe from the child to the kernel and then sending the kernel a SIGTRAP.	
*/
		// This section is for if there are new processes that need to be forked and execled. 
		running -> interrupts ++;
		if (!new_list.empty()) 
		{
			running -> state = READY;
			PCB *nextProc = new_list.front();

			pid_t pid = fork();
			if ( pid < 0 )
			{ 
				perror("fork failed"); 
			}
			else if ( pid == 0) // in child process
			{

				assert (close (nextProc->p2k[READ_END]) == 0);
        			assert (close (nextProc->k2p[WRITE_END]) == 0);

        			// assign fildes 3 and 4 to the pipe ends in the child
        			assert (dup2 (nextProc->p2k[WRITE_END], 3) >= 0);
        			assert (dup2 (nextProc->k2p[READ_END], 4) >= 0);
				execl (new_list.front()->name, new_list.front()->name, NULL);
                		perror ("execl");
			}
			else // in parent process
			{
				nextProc->ppid = getpid();
				nextProc->state = RUNNING;
				running->switches++;
				nextProc->pid = pid;
				nextProc->started = sys_time;
				
			}
			new_list.pop_front();
			processes.push_back(nextProc);
			running = processes.back();
			return running;
		}

//		Two for loops isnt completely needed here. I started with one mindset and though about switching it to one.
//		This is the round robin section.
//		The break also should save a tiny amount of time. 
		list<PCB*>::iterator it; 
		for ( it = processes.begin(); it != processes.end(); it++)
		{
	
			if ( (*it)-> state == RUNNING)
			{
				processes.push_back(running);
				processes.back()->state = READY;
				processes.erase(it);
				break;
			}
		}
		for ( it = processes.begin(); it != processes.end(); it++)
		{		
			if ( (*it) -> state == READY) 
			{
				if ( (*it)->pid != running->pid)
				{
					(running)->switches++;
				}
				(*it)->state = RUNNING;
				running = *it;
				break;
			}
			else {
				running = idle;
			}
		}
		
    return running;
}

void scheduler (int signum)
{
    assert (signum == SIGALRM);
    sys_time++;

    PCB* tocont = choose_process();

    dprintt ("continuing", tocont->pid);
    if (kill (tocont->pid, SIGCONT) == -1)
    {
        perror ("kill");
        return;
    }
}
// Potential bug, Every so often "kill no such process" happens. 
void process_done (int signum)
{

	cout << "Process " << running->name << " was:\n";
	cout << "interrupted " << running->interrupts << " time(s)\n";
	cout << "switched " << running->switches << " time(s)\n";
	cout << "and ran for " << sys_time - running->started << " second(s)\n";
	running->state = TERMINATED;
	running = idle;

    assert (signum == SIGCHLD);
    WRITE("---- entering child_done\n");

    for (;;)
    {

        int status, cpid;
        cpid = waitpid (-1, &status, WNOHANG);

        if (cpid < 0)
        {
            WRITE("cpid < 0\n");
            kill (0, SIGTERM);
        }
        else if (cpid == 0)
        {
            WRITE("cpid == 0\n");
            break;
        }
        else
        {
            char buf[10];
            assert (eye2eh (cpid, buf, 10, 10) != -1);
            WRITE("process exited:");
            WRITE(buf);
            WRITE("\n");
	    // I am not sure how we should be setting NUM_CHILDREN so im leaving it out for now.
            //child_count++;
            //if (child_count == NUM_CHILDREN)
            //{
            //    kill (0, SIGTERM);
            //}
        }
    }

    if (kill (running->pid, SIGSTOP) == -1)
    {
        perror ("kill");
        return;
    }

 
    WRITE("---- leaving child_done\n");
}


void process_trap(int signum) 
{
/*
Instructions: You'll need to create a SIGTRAP ISR that reads the request and sends back a response. Implement at least the listing of the names of all the current processes and the system time, and implement a child process that requests both.
*/
    assert (signum == SIGTRAP);
    WRITE("---- entering process_trap\n");

    /*
    ** poll all the pipes as we don't know which process sent the trap, nor
    ** if more than one has arrived.
    */
	for (int i = 0; i < NUM_PIPES; i+=2)
    	{
        	char buf[1024];
        	int num_read = read (running->p2k[READ_END], buf, 1023);
		//cout << running->p2k[0] << "\n";
        	if (num_read > 0)
        	{	

			buf[num_read] = '\0';
            		WRITE("kernel read: ");
            		WRITE(buf);
            		WRITE("\n");
			
			// respond
            		const char *message = "from the kernel to the process \n";
            		if (write (running->k2p[WRITE_END], message, strlen (message)) != -1)
			{
				perror("write");
			}
			

				// I should of created a seperate method to handle the string creations but it was giving
				// me trouble and I was running out of time
				//
				if ( strcmp("pt", buf) == 0 || strcmp("tp", buf) == 0)
				{
					WRITE("Child requested the process list and system time \n");
					char message[1024];
					char time[1024];

					strcpy(message, "The list of processes is: ");
					list<PCB*>::iterator it; 
					for ( it = processes.begin(); it != processes.end(); it++)
					{
						strcat(message, (*it)->name);
						strcat(message, " ");
					}
					strcat(message, "\n");
					
					strcat(message, "The system time is: ");
					sprintf(time, "%d", sys_time);
					strcat(message, time);
					strcat(message, "\n");
					if (write (running->k2p[WRITE_END], message, strlen (message)) == -1)
					{
						perror("write");
					}
					
				}

				else if (buf[0] == 'p')
				{
					WRITE("Child requested the process list\n");
					// Bob helped me with this section of code, "Dong Lee". All used was the string concatination. 
					// I already had the processes talking to each other through the pipes. 
					char toChild[1024];
					strcpy(toChild, "The list of processes is: ");
		
					list<PCB*>::iterator it; 
					for ( it = processes.begin(); it != processes.end(); it++)
					{
						strcat(toChild, (*it)->name);
						strcat(toChild, " ");
					}
					strcat(toChild, "\n");
					if (write (running->k2p[WRITE_END], toChild, strlen (toChild)) == -1)
					{
						perror("write");
					}
					
				}
			
				else if (buf[0] == 't')
				{
					WRITE("Child Requested the system time \n");
					// Again bob helped
					char mess[1024];
					char time[1024];
					strcpy(mess, "The system time is: ");
					sprintf(time, "%d", sys_time);
					strcat(mess, time);
					strcat(mess, "\n");
					if (write (running->k2p[WRITE_END], mess, strlen (mess)) == -1)
					{
						perror("write");
					}
				}
        	}
    	}
    	WRITE("---- leaving process_trap\n");
}
/*
** stop the running process and index into the ISV to call the ISR
*/
void ISR (int signum)
{
	if ( signum != SIGCHLD) {

    if (kill (running->pid, SIGSTOP) == -1)
    {
        perror ("kill");
        return;
    }
    dprintt ("stopped", running->pid);

	}
    ISV[signum](signum);
}

/*
** set up the "hardware"
*/
void boot (int pid)
{
    ISV[SIGALRM] = scheduler;       create_handler (SIGALRM, ISR);
    ISV[SIGCHLD] = process_done;    create_handler (SIGCHLD, ISR);
    ISV[SIGTRAP] = process_trap;    create_handler (SIGTRAP, ISR);
    // start up clock interrupt
    int ret;
    if ((ret = fork ()) == 0)
    {
        // signal this process once a second for three times
        send_signals (SIGALRM, pid, 1, NUM_SECONDS);

        // once that's done, really kill everything...
        kill (0, SIGTERM);
    }

    if (ret < 0)
    {
        perror ("fork");
    }
}

void create_idle ()
{
    int idlepid;

    if ((idlepid = fork ()) == 0)
    {
        dprintt ("idle", getpid ());

        // the pause might be interrupted, so we need to
        // repeat it forever.
        for (;;)
        {
            dmess ("going to sleep");
            pause ();
            if (errno == EINTR)
            {
                dmess ("waking up");
                continue;
            }
            perror ("pause");
        }
    }
    idle = new (PCB);
    idle->state = RUNNING;
    idle->name = "IDLE";
    idle->pid = idlepid;
    idle->ppid = 0;
    idle->interrupts = 0;
    idle->switches = 0;
    idle->started = sys_time;
}


//Luke Smith helped me with this part, its a mirror of create_idle
 
void create_process(const char *program) {

		PCB *proc = new (PCB);
		proc->state = NEW;
		proc->name = program;
		proc->ppid = 0;
		proc->interrupts = 0;
		proc->switches = 0;
		proc->started = sys_time;
			
        	assert (pipe (proc->p2k) == 0);
       		assert (pipe (proc->k2p) == 0);

        	assert (fcntl (proc->p2k[READ_END], F_SETFL,
            		fcntl(proc->p2k[READ_END], F_GETFL) | O_NONBLOCK) == 0);


		new_list.push_back(proc);
}

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------

	for (int i = 1; i < argc; i++) {
		create_process(argv[i]);
	}
	
    //--------------------------------------------------------------------------

    int pid = getpid();
    dprintt ("main", pid);

    sys_time = 0;

    boot (pid);

    // create a process to soak up cycles
    create_idle ();
    running = idle;

    cout << running;

    // we keep this process around so that the children don't die and
    // to keep the IRQs in place.
    for (;;)
    {
        pause();
        if (errno == EINTR) { continue; }
        perror ("pause");
    }

}
