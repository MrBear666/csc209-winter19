#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants that determine that address ranges of different memory regions

#define GLOBALS_START 0x400000
#define GLOBALS_END   0x700000
#define HEAP_START   0x4000000
#define HEAP_END     0x8000000 
#define STACK_START 0xfff000000

int main(int argc, char **argv) {
    
    FILE *fp = NULL;
    unsigned long cur_marker;
    char references;
    int num_I = 0;
    int num_M = 0;
    int num_S = 0;
    int num_L = 0; 
    int loc_G = 0;
    int loc_H = 0;
    int loc_S = 0;

    if(argc == 1) {
        fp = stdin;

    } 
    else if(argc == 2) {
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }

    } else {
        fprintf(stderr, "Usage: %s [tracefile]\n", argv[0]);
        exit(1);
    }

    while (fscanf(fp, "%c,%lx ", &references, &cur_marker) != EOF) {
            if (references != 'I') {
                //check if reference is not "I"
                if (GLOBALS_START < cur_marker && cur_marker < GLOBALS_END) {
                    loc_G ++;
                }
                if (HEAP_START < cur_marker && cur_marker < HEAP_END) {
                    loc_H ++;
                } 
                if (STACK_START < cur_marker) {
                    loc_S ++;
                }
            }

            //chech reference type
            if (references == 'I') {
                num_I ++;
            } 
            else if (references == 'M') {
                num_M ++;
            }
            else if (references == 'S') {
                num_S ++;
            }
            else if (references == 'L') {
                num_L ++;
            }
        }
    fclose(fp);

    /* Use these print statements to print the ouput. It is important that 
     * the output match precisely for testing purposes.
     * Fill in the relevant variables in each print statement.
     * The print statements are commented out so that the program compiles.  
     * Uncomment them as you get each piece working.
     */
    
    printf("Reference Counts by Type:\n");
    printf("    Instructions: %d\n", num_I);
    printf("    Modifications: %d\n", num_M);
    printf("    Loads: %d\n", num_L);
    printf("    Stores: %d\n", num_S);
    printf("Data Reference Counts by Location:\n");
    printf("    Globals: %d\n", loc_G);
    printf("    Heap: %d\n", loc_H);
    printf("    Stack: %d\n", loc_S);

    return 0;
}
