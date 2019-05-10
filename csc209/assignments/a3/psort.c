#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "helper.h"

/* This program takes 7 arguments. It reads structs from 
*  input file, and sort it by qsort in each child process, 
*  then child precess wirte to pipe. Parent reads from pipe 
*  and sort it, then write those structs into the output 
*  file as ordered(least to largest).
*  
*  -n as the number of processes, 
*  -f as input file. 
*  -o as output file. 
*/
int main(int argc, char *argv[]) {

    extern char *optarg;
    int ch, num_processes, status;
    char *infile = NULL, *outfile = NULL;

    if (argc != 7) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
        exit(1);
    }

    /* read in arguments */
    while ((ch = getopt(argc, argv, "n:f:o:")) != -1) {
        switch(ch) {
        case 'n':
            num_processes = (int) strtol(optarg, NULL, 10);
            break;
        case 'f':
            infile = optarg;
            break;
        case 'o':
            outfile = optarg;
            break;
        default:
            fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
            exit(1);
        }
    }

    // zero process the exit with code 0
    if (num_processes <= 0){
        exit(0);
    }
    // find the number of struct we need to process
    int num_structs = get_file_size(infile) / sizeof(struct rec); 

    if (num_structs == 0) {
        exit(0); 
    }
    else if (num_structs < num_processes) {
        num_processes = num_structs;
    }

    // the number of structs that each process need to process
    int *struct_distribute = divide_up_words(num_structs, num_processes); // need to free

    // initialize pipe
    int pipe_fd[num_processes][2]; 

    for (int i = 0; i < num_processes; i++) {
        if ((pipe(pipe_fd[i])) == -1) {
            perror("pipe");
            exit(1);
        }

        int result = fork();
        if (result < 0) {
            perror("fork");
            exit(1);
        }
        else if (result == 0) { // child
            // child only writes to the pipe so close reading end
            close_pipe_error_checking(pipe_fd[i][0], READ);

            // close reading ends of all previously forked children
            for (int child_no = 0; child_no < i; child_no++) {
                close_pipe_error_checking(pipe_fd[child_no][0], READ);
            }

            // read from file and qsort the read structs, then write them to pipe
            sorted_array_to_pipe(struct_distribute, infile, i, pipe_fd[i][1]);

            // done writing with the pipe so close it and check err
            close_pipe_error_checking(pipe_fd[i][1], WRITE);

            free(struct_distribute);
            exit(0);
        }
        else { // parent
            // close writng end of pipe and checking error
            close_pipe_error_checking(pipe_fd[i][1], WRITE);
        }
    }

    // read structs from pipes and sort them, 
    // then fwrite them to output file in increasing order. 
    merge_func(num_processes, struct_distribute, pipe_fd, num_structs, outfile);

    // Wait
    int pid;
    for (int wait_num = 0; wait_num < num_processes; wait_num++) {
        if ( (pid = wait(&status)) != -1) {
            if (!(WIFEXITED(status))) { //child does not terminate normally
                fprintf(stderr, "Child terminated abnormally\n");
            }
        }
        else { // wait fail
            perror("Wait");
        }
    }
    free(struct_distribute);
}
