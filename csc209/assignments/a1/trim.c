#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {
    
    if(argc != 3) {
        fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
        exit(1);
    }

    // Addresses should be stored in unsigned long variables
    unsigned long start_marker, end_marker, cur_marker;
    char references;
    int n;

    FILE *fp1 = fopen(argv[1], "r");
    FILE *fp2 = fopen(argv[2], "r");

    //get start marker & end marker
    if (fp2 != NULL) {
        fscanf(fp2, "%lx %lx", &start_marker, &end_marker);
    }
    else {
        perror("fopen");
        exit(1);
    }
    fclose(fp2);
    int pr_flag = 0;

    if (fp1 != NULL) {
        while (fscanf(fp1, " %c %lx,%d", &references, &cur_marker, &n) != EOF) {    
            if (cur_marker == start_marker) {
                pr_flag = 1;
            }
            else if (cur_marker == end_marker) {
                pr_flag = 0;
            }
            //print the content in range.
            if ((pr_flag == 1) && (cur_marker != start_marker)) {
                printf("%c,%#lx\n", references, cur_marker);
            }
        }
    }
    else {
        perror("fopen");
        exit(1);
    }
    
    return 0;
} 

