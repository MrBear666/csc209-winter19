#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "helper.h"

#define UPPER 30000


/*
 * Return a randomly generated number, uniformly distributed between
 * lower and upper.
 */
int uniform(int lower,int upper) {
    return (int) (floor ( drand48() * (upper - lower + 1) ) + lower);
}

/* This program takes as input a file containing one word per line.  
 * It uses the each word together with a randomly generated frequency count 
 * to create a struct that is written to the output file.
 * The result is a binary file in the correct format to use as input to
 * psort.
 * 
 * To compile the program the math library must be linked:
 *          gcc -Wall -g -std=gnu99 -o mkwords mkwords.c -lm
 */

int main(int argc, char *argv[]) {
    extern char *optarg;
    int ch;
    FILE *infp, *outfp;
    struct rec record;
    char *infile = NULL, *outfile = NULL;

    if (argc != 5) {
        fprintf(stderr, "Usage: mkwords -f <input file name> -o <output file name>\n");
        exit(1);
    }

    /* read in arguments */
    while ((ch = getopt(argc, argv, "f:o:")) != -1) {
        switch(ch) {
        case 'f':
            infile = optarg;
            break;
        case 'o':
            outfile = optarg;
            break;
        default:
            fprintf(stderr, "Usage: mkwords -f <input file name> -o <output file name>\n");
            exit(1);
        }
    }

    /* seed the random number generator */
    
    srand48(time(NULL)); 

    if ((infp = fopen(infile, "r")) == NULL) {
        fprintf(stderr, "Could not open %s\n", infile);
        exit(1);
    }
    if ((outfp = fopen(outfile, "w")) == NULL) {
        fprintf(stderr, "Could not open %s\n", outfile);
        exit(1);
    }

    /* read a word from the input file, and make up a frequency for it */
	/* Expects the input file to have one word per line */
    while ((fgets(record.word, sizeof(record.word), infp)) != NULL) {
		// remove the newline character
        record.word[strlen(record.word) - 1] = '\0';
        record.freq = uniform(0, UPPER);

        if ((fwrite(&record, sizeof(record), 1, outfp)) != 1) {
            fprintf(stderr, "Could not write to %s\n", outfile);
        }
    }

    /* Close both files. */
    if (fclose(infp)) {
        perror("fclose");
        exit(1);
    }

    if (fclose(outfp)) {
        perror("fclose");
        exit(1);
    }

    return 0;
}
