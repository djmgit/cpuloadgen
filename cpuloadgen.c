#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

#define exitWithError(msg) do {perror(msg); exit(EXIT_FAILURE);} while(0)

double getPreciseTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double usecParsed = (double)(tv.tv_usec);

    // convert usec to decimal
    while (usecParsed > 1) {
        usecParsed /= 10;
    }

    return (double)(tv.tv_sec + usecParsed);
}

int precisionSleep(double sleepTime) {
    struct timespec tv;
    tv.tv_sec = (time_t)sleepTime;
    tv.tv_nsec = (long)((sleepTime - tv.tv_sec) * 1e+9);

    // handle broken sleep due to interrupt
    while(1) {
        int rval = nanosleep(&tv, &tv);
        if (rval == 0) {
            return 0;
        } else if (errno == EINTR) {
            continue;
        } else {
            return rval;
        }
    }
}

void doWork() {
    long num = 77l * 77l * 77l * 77l * 77l;
    sqrt(num);
}

void pinCpu(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);

    if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
        exitWithError("sched_setaffinity failed");
    }
}

void generateCPULoad(int totalTime, float util, int cpu) {
    if (cpu != -1) {
        pinCpu(cpu);
    }

    double startTime = getPreciseTimeStamp();
    double uitlFraction = (double)(util / 100);
    
    for (size_t i = 0; i < totalTime; i++) {
        while ((getPreciseTimeStamp() - startTime) < uitlFraction) {
            doWork();
        }
        precisionSleep(1-uitlFraction);
        startTime++;
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    size_t numCPUOnline = sysconf(_SC_NPROCESSORS_ONLN);
    int c;
    int totalTime = 60;
    int util = 25;
    pid_t pid;
    int status;
    int numCPUs = 0;

    int *cpus = (int*)malloc(sizeof(int) * numCPUOnline);
    memset(cpus, 0, numCPUOnline);
    
    while ((c = getopt (argc, argv, "t:p:c:")) != -1) {
        switch (c) {
            case 't':
                totalTime = atoi(optarg);
                break;
            case 'p':
                util = atoi(optarg);
                break;
            case 'c':
                cpus[atoi(optarg)] = 1;
                numCPUs += 1;
                break;
            default:
                abort();
        }
    }

    for (size_t i = 0; i < numCPUOnline; i++) {
        if (cpus[i] == 0 && numCPUs > 0) {
            continue;
        }
        pid_t pid = fork();
        if (pid < 0) {
            exitWithError("Failed to fork process");
        }
        if (pid == 0) {
            if (numCPUs > 0) {
                // we are generating load on particular CPUs, hence
                // pinning is required.
                generateCPULoad(totalTime, util, i);
            } else {
                // we are generating load on all CPUs, do not
                // pin since we have #CPU number of processes.
                generateCPULoad(totalTime, util, -1);
            }
        }
    }

    while ((pid = wait(&status)) > 0);
}

