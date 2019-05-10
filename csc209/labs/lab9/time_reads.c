/* The purpose of this program is to practice writing signal handling
 * functions and observing the behaviour of signals.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Message to print in the signal handling function. */
#define MESSAGE "%ld reads were done in %ld seconds.\n"

/* Global variables to store number of read operations and seconds elapsed. 
 */
long num_reads, seconds;

/*
 * Signal handler for SIGPROF
 */
void handler(int code) {
    printf(MESSAGE, num_reads, seconds);
    exit(0);
}


/* The first command-line argument is the number of seconds to set a timer to run.
 * The second argument is the name of a binary file containing 100 ints.
 * Assume both of these arguments are correct.
 */

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: time_reads s filename\n");
        exit(1);
    }
    //initialize fields
    seconds = strtol(argv[1], NULL, 10);
    num_reads = 0;
    struct sigaction newsa;
    struct itimerval timer;

    FILE *fp;
    if ((fp = fopen(argv[2], "r")) == NULL) {
      perror("fopen");
      exit(1);
    }

    // SIGPROF
    newsa.sa_flags = 0;
    newsa.sa_handler = handler;
    sigemptyset(&newsa.sa_mask);
    sigaction(SIGPROF, &newsa, NULL);

    // Set timer
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;
    if (setitimer(ITIMER_PROF, &timer, NULL) < 0) {
        fprintf(stderr, "There is error in getitimer\n");
        exit(1);
    }


    /* In an infinite loop, read an int from a random location in the file,
     * and print it to stderr.
     */
    int n;
    for (;;) {
      int location = random() % 100;
        if (fseek(fp, sizeof(int) * location, SEEK_SET) != 0) {
            perror("fseek");
            exit(1);
        }
        if (fread(&n, 1, sizeof(int), fp) <= 0) {
            perror("fread");
            exit(1);
        }
        fprintf(stderr, "%d\n", n);
        num_reads++;
    }
    return 1; // something is wrong if we ever get here!
}
