#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Write random integers (in binary) to a file with the name given by the command-line
 * argument.  This program creates a data file for use by the time_reads program.
 */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: write_test_file filename\n");
        exit(1);
    }

    FILE *fp;
    if ((fp = fopen(argv[1], "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    // TODO: complete this program according its description above.
    for (int n = 0; n < 100; n++) {
        int i = random() % 100;
        if (fwrite(&i, 1, sizeof(int), fp) != sizeof(int)) {
            perror("fwrite");
            exit(1);
        }
    }




    fclose(fp);
    return 0;
}
