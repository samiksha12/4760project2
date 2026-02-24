#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

using namespace std;

const int BUFF_SZ = sizeof(int) * 2;
int shm_key;
int shm_id;
int *customClock = nullptr;
int launchedProcess = 0;
int terminatedProcess = 0;

const int BILLION = 1000000000;
const int MAXIMUM_PROCESS = 20;

// PCB Structure

struct PCB
{
    int occupied;          // either true or false
    pid_t pid;             // process id of this child
    int startSeconds;      // time when it was forked
    int startNano;         // time when it was forked
    int endingTimeSeconds; // estimated time it should end
    int endingTimeNano;    // estimated time it should end
};
struct PCB processTable[MAXIMUM_PROCESS];

void incrementClock()
{
    customClock[1] += 10000000;

    while (customClock[1] >= BILLION)
    {
        customClock[0]++;
        customClock[1] -= BILLION;
    }
}

// check if time passed
bool timePassed(int sec, int nano)
{
    if (customClock[0] > sec)
        return true;
    if (customClock[0] == sec && customClock[1] >= nano)
        return true;

    return false;
}

// Cleaning the shared memory
void cleanup()
{
    for (int i = 0; i < MAXIMUM_PROCESS; i++)
    {
        if (processTable[i].occupied)
        {
            kill(processTable[i].pid, SIGTERM);
        }
    }

    if (customClock != nullptr)
        shmdt(customClock);
    customClock = 0;
    if (shm_id != -1)
        shmctl(shm_id, IPC_RMID, NULL);
}

// Print Process table
void printProcessTable()
{
    cout << "\nOSS PID: " << getpid()
         << " Seconds: " << customClock[0] << " Nanoseconds: " << customClock[1] << "\n";
    cout << "Entry \t Occupied \t PID \t StartSec \t StartNano \t EndSec \t EndNano \n";
    for (int i = 0; i < MAXIMUM_PROCESS; i++)
    {
        if (processTable[i].occupied)
        {
            cout << i << "\t" << "1 \t" << processTable[i].pid << "\t" << processTable[i].startSeconds << "\t" << processTable[i].startNano << "\t" << processTable[i].endingTimeSeconds << "\t" << processTable[i].endingTimeNano << "\n";
        }
        else
        {
            cout << i << "\t" << "0" << "\n";
        }
    }
}

void printHelp()
{
    cout << "Usage: oss [-h] [-n proc] [-s simul] [-t iter]\n"
         << "-n proc  total number of user processes (required, 1-100)\n"
         << "-s simul max simultaneous processes (default 2, 1-15)\n"
         << "-t iter  iterations per user(default 3, 0-100)\n";
}

// signal handler
void signalHandler(int sig)
{
    cout << "\nOSS: Caught signal " << sig << ", cleaning up...\n";
    cleanup();
    exit(1);
}

// Main Function
int main(int argc, char *argv[])
{

    int n = -10;    // Since it is required I gave a dummy number, for requirement condition I am going to use ahead
    int s = 2;      // Default value is 2
    double t = 3.0; // Default value is 3seconds 0nanosecond
    double interval = 0.2;

    int option;
    while ((option = getopt(argc, argv, "hn:s:t:i:")) != -1)
    {
        switch (option)
        {
        case 'h':
            printHelp();
            return 0;
        case 'n':
            n = atoi(optarg); // optarg is the global variable set by getopt when a parameter with a value has been provided
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 't':
            t = atof(optarg);
            break;
        case 'i':
            interval = atof(optarg);
            break;
        default:
            printHelp();
            return 1;
        }
    }
    if (n == -10)
    {
        cerr << "Error: -n (number of processes) is required. \n";
        return 1;
    }
    if (n < 1 || n > 100)
    {
        cerr << "Error: -n must be between 1 and 100.\n";
        return 1;
    }

    if (s < 1 || s > 15)
    {
        cerr << "Error: -s must be between 1 and 15.\n";
        return 1;
    }

    if (t < 0.0 || t > 60.0)
    {
        cerr << "Error: -t must be between 0 and 60.\n";
        return 1;
    }

    if (s > n)
    {
        s = n; // just in case simultaneous is greater than number of process
    }
    cout << "OSS starting, PID: " << getpid() << "\n";
    cout << "-n " << n << " -s " << s
         << " -t " << t << " -i " << interval << "\n";

    // Shared Memory
    int shm_key = ftok("oss.cpp", 0);
    if (shm_key <= 0)
    {
        fprintf(stderr, "Parent:... Error in ftok\n");
        exit(1);
    }

    int shm_id = shmget(shm_key, BUFF_SZ, 0700 | IPC_CREAT);
    if (shm_id <= 0)
    {
        fprintf(stderr, "Parent:... Error in shmget\n");
        exit(1);
    }

    customClock = (int *)shmat(shm_id, 0, 0);
    if (customClock == (int *)-1)
    {
        fprintf(stderr, "Parent:... Error in shmat\n");
        exit(1);
    }
    customClock[0] = 0;
    customClock[1] = 0;

    // Setup signals
    signal(SIGALRM, signalHandler);
    signal(SIGINT, signalHandler);
    alarm(60);

    // Using sec and nano is throwing me error no matter what I do
    //  int *sec = &(customClock[0]);
    //  int *nano = &(customClock[1]);
    //  *sec = *nano = 0;

    // initializing PCB
    for (int i = 0; i < MAXIMUM_PROCESS; i++)
        processTable[i].occupied = false;

    int activeWorkers = 0;
    int lastLaunchSec = 0;
    int lastLaunchNano = 0;
    int lastPrintSec = 0;
    int lastPrintNano = 0;

    while (terminatedProcess < n)
    {
        incrementClock();

        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0)
        {
            terminatedProcess++;
            activeWorkers--;
            for (int i = 0; i < MAXIMUM_PROCESS; i++)
            {
                if (processTable[i].pid == pid)
                {
                    processTable[i].occupied = 0;
                    break;
                }
            }
        }

        bool intervalPassed = false;
        int intervalSec = (int)interval;
        int intervalNano = (interval - intervalSec) * BILLION;
        int tSec = (int)t;
        int tNano = (t - tSec) * BILLION;

        if (timePassed(lastLaunchSec + intervalSec, lastLaunchNano + intervalNano))
        {
            intervalPassed = true;
        }
        if (launchedProcess < n && activeWorkers < s && intervalPassed)
        {

            for (int i = 0; i < MAXIMUM_PROCESS; i++)
            {
                if (!processTable[i].occupied)
                {
                    // launch worker
                    pid_t child_pid = fork();
                    if (child_pid == 0)
                    {
                        // in child
                        char tSecString[10];
                        char tNanoString[10];
                        std::strcpy(tSecString, std::to_string(tSec).c_str());
                        std::strcpy(tNanoString, std::to_string(tNano).c_str());
                        execlp("./worker", "./worker", tSecString, tNanoString, (char *)0);

                        fprintf(stderr, "Error in exec after fork\n");
                        exit(1);
                    }
                    processTable[i].occupied = 1;
                    processTable[i].pid = child_pid;
                    processTable[i].startSeconds = customClock[0];
                    processTable[i].startNano = customClock[1];
                    processTable[i].endingTimeSeconds = customClock[0] + tSec;
                    processTable[i].endingTimeNano = customClock[1] + tNano;

                    launchedProcess++;
                    activeWorkers++;

                    lastLaunchSec = customClock[0];
                    lastLaunchNano = customClock[1];
                    break;
                }
            }
        }
        // printing processtable every half second

        if (timePassed(lastPrintSec, lastPrintNano + 500000000))
        {
            printProcessTable();

            lastPrintSec = customClock[0];
            lastPrintNano = customClock[1];
        }
    }

    cleanup();
    cout << "\nOSS terminating\n";
    cout << launchedProcess << " workers launched and terminated\n";

    return 0;
}