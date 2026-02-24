#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace std;

const int BUFF_SZ = sizeof(int) * 2;
int shm_key;
int shm_id;
const int BILLION = 1000000000;
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "Usage: ./worker <seconds> <nanoseconds>" << endl; // We need 2 arguments ./user and number of iteration
        return 1;
    }
    int seconds = atoi(argv[1]);
    int nanoseconds = atoi(argv[2]);

    int shm_key = ftok("oss.cpp", 0);
    if (shm_key <= 0)
    {
        fprintf(stderr, "Child:... Error in ftok\n");
        exit(1);
    }

    int shm_id = shmget(shm_key, BUFF_SZ, 0700);
    if (shm_id <= 0)
    {
        fprintf(stderr, "child:... Error in shmget\n");
        exit(1);
    }

    int *clock = (int *)shmat(shm_id, 0, 0);
    if (clock == (int *)-1)
    {
        fprintf(stderr, "Child:... Error in shmat\n");
        exit(1);
    }

    int *sec = &(clock[0]);
    int *nano = &(clock[1]);

    int termTimeSec = *sec + seconds;
    int termTimeNano = *nano + nanoseconds;

    if (termTimeNano >= BILLION)
    {
        termTimeSec++;
        termTimeNano -= BILLION;
    }
    int startSec = *sec;

    printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
    printf("SysClockS:%d SysClockNano:%d TermTimeS:%d TermTimeNano:%d\n", *sec, *nano, termTimeSec, termTimeNano);

    printf("--Just Starting\n");

    int lastPrintedSecond = *sec;
    while (true)
    {
        if (*sec > termTimeSec || (*sec == termTimeSec && *nano >= termTimeNano))
        {
            printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
            printf("SysClockS:%d SysClockNano:%d TermTimeS:%d TermTimeNano:%d\n", *sec, *nano, termTimeSec, termTimeNano);
            printf("--Terminating\n");
            break;
        }
        if (*sec > lastPrintedSecond)
        {
            printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
            printf("SysClockS:%d SysClockNano:%d TermTimeS:%d TermTimeNano:%d\n", *sec, *nano, termTimeSec, termTimeNano);
            printf("--%d seconds have passed since starting\n", *sec - startSec);
            lastPrintedSecond = *sec;
        }
    }

    shmdt(clock);
    return 0;
}