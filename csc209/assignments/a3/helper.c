#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"

/* return an int that is how many bytes the file is */
int get_file_size(char *filename) {
    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *) rec1;
    struct rec *r2 = (struct rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

/*  
*   Return an array pointer which locate at heap, contains
*   the number of struct that each process should process by
*   dividing up the number of structs as evenly as possible by 
*   the number of processes.
*/
int *divide_up_words(int num_structs, int num_processes) {
    int *result = malloc(num_processes * sizeof(int)); 
    if (result == NULL) { //check err
        perror("malloc");
        exit(1);
    }
    
    int min_num_structs = num_structs / num_processes;
    int remain = num_structs % num_processes;

    for (int i = 0; i < num_processes; i++) {
        if (i < remain) {
            result[i] = min_num_structs + 1;
        } 
        else {
            result[i] = min_num_structs;
        }
    }
    return result;
}


/*  close pipe reading or writing end, and error checking 
*/
void close_pipe_error_checking(int int_pipe, int type) {
    switch(type) {
        case READ:
            if (close(int_pipe) == -1) {
                perror("close reading end of pipe");
                exit(1);
            }
            break;
        case WRITE:
            if (close(int_pipe) == -1) {
                perror("close writing end of pipe");
                exit(1);
            }
            break;
    }
}


/*  read or write error checking 
*/
void read_write_error_checking(int check_num, int type, int bytes_wanted) {
    switch(type) {
        case READ:
            if (check_num == -1) {
                perror("read Error");
                exit(1);
            }
            else if (check_num != bytes_wanted) {
                fprintf(stderr, "read %d bytes, but want %d bytes.\n", check_num, bytes_wanted);
                exit(1);
            }
            break;
        case WRITE:
            if (check_num == -1) {
                perror("write Error");
                exit(1);
            }
            else if (check_num != bytes_wanted) {
                fprintf(stderr, "write %d bytes, but want %d bytes.\n", check_num, bytes_wanted);
                exit(1);
            }
            break;
    }
}


/*  Read num of structs from infp file, then put these 
*   structs in rec_arr. Then return the arr.
*/
struct rec *struct_arr_form_file(int num, FILE *infp) {

    int check;
    struct rec *rec_arr = malloc(sizeof(struct rec)* num); //free
    if (rec_arr == NULL) { //check err
        perror("malloc");
        exit(1);
    }

    if ( (check = fread(rec_arr, sizeof(struct rec), num, infp)) != num) {
        fprintf(stderr, "fread: Cannot read file.\n");
        exit(1);        
    }
    return rec_arr;
}


/*  helper for child processes: read structs from file and sort those 
*   structs then write them into pipe in each child process.
*
*   struct_distribute: contains how many structs that each process need to read from file;
*   infp: the input file;
*   ind_child: index of child;
*   pipe_fd: int of pipe_fd
*/
void sorted_array_to_pipe(int *struct_distribute, char *infile, int ind_child, int pipe_fd) {

    FILE *infp;
    int position = 0;
    int check;

    if ((infp = fopen(infile, "rb")) == NULL) {
        fprintf(stderr, "Could not open %s\n", infile);
        exit(1);
    }

    // set the starting position to read from file
    for (int i = 0; i < ind_child; i++) {
        position += (struct_distribute[i]) * sizeof(struct rec);
    }
    if ( (check = fseek(infp, position, SEEK_SET)) == -1) {
        perror("fseek");
        exit(1);
    }

    // read structs from file and write to array, rec_arr.
    struct rec *rec_arr = struct_arr_form_file(struct_distribute[ind_child], infp); // free

    if (fclose(infp)) {
        perror("fclose");
        exit(1);
    }
    // sort by qsort
    qsort(rec_arr, struct_distribute[ind_child], sizeof(struct rec), compare_freq);
    // write to pipe
    for (int k = 0; k < struct_distribute[ind_child]; k++) {
        check = write(pipe_fd, &rec_arr[k], sizeof(struct rec));
        read_write_error_checking(check, WRITE, sizeof(struct rec));
    }
    free(rec_arr);
}


/*  set up and creating a marge array by combining the first element of each pipe
*/
struct rec *set_up_marge_arr(int num_processes, int *struct_distribute, int pipe_fd[][2]) {

    int check;
    struct rec *marge_arr = malloc(num_processes * (sizeof(struct rec)));
    
    for (int ind_p = 0; ind_p < num_processes; ind_p++) {
        // 0 means no struct in pipe, so close it
        if (struct_distribute[ind_p] != 0) {
            check = read(pipe_fd[ind_p][0], &marge_arr[ind_p], sizeof(struct rec));
            read_write_error_checking(check, READ, sizeof(struct rec));

            struct_distribute[ind_p] -= 1;
            if (struct_distribute[ind_p] == 0) {
                close_pipe_error_checking(pipe_fd[ind_p][0], READ);
            }   
        }
    }
    return marge_arr;
}


/*  helper for parent processes: read structs from each pipe and use linear 
    sort those structs then write them into file

    num_processes: the number of processes; 
    struct_distribute: contains how many structs that each process need to read from file;
    pipe_fd: pipe int;
    num_structs: number of structs we need to process in total;
    outfp: the output file;
*/
void merge_func(int num_processes, int *struct_distribute, int pipe_fd[][2], int num_structs, char *outfile) {

    FILE *outfp;
    if ((outfp = fopen(outfile, "wb")) == NULL) {
        fprintf(stderr, "Could not open parent %s\n", outfile);
        exit(1);
    }
    // step 0: set up and creating a marge array by combining the first element of each pipe
    struct rec *marge_arr = set_up_marge_arr(num_processes, struct_distribute, pipe_fd); // free

    // setp 1: process the least element and fwrite to file and repeat num_structs times;
    for (int acc_struct = 0; acc_struct < num_structs; acc_struct++) {
        // step 1.1: set a fake least struct in each iteration
        struct rec least;
        int ind_least;
        for (int index = 0; index < num_processes; index++) {
            // -1 means no useful struct at marge_arr[index]
            if (struct_distribute[index] != -1) {
                least = marge_arr[index];
                ind_least = index;
                break;
            }
        }
        // step 1.2: find the real least by compare_freq
        for (int arr_index = 0; arr_index < num_processes; arr_index++) {
            // != -1 means: there is an useful struct at marge_arr[arr_index]
            if (struct_distribute[arr_index] != -1) {
                if (compare_freq(&least, &marge_arr[arr_index]) == 1) {
                    least = marge_arr[arr_index];
                    ind_least = arr_index;
                }
            }
        }
        // step 1.3: fwrite the least to file
        int check = fwrite(&least, sizeof(struct rec), 1, outfp);
        if (check != 1) { //check err
            fprintf(stderr, "Error: Cannot write file\n");
            exit(1);
        }
        // step 1.4: fill the gap of marge_arr if pipe has struct
        if (struct_distribute[ind_least] != 0) {
            check = read(pipe_fd[ind_least][0], &marge_arr[ind_least], sizeof(struct rec));
            read_write_error_checking(check, READ, sizeof(struct rec));
        }
        // step 1.5: update number of rest struct in the pipe 
        // and close pipe if 0 struct in pipe
        struct_distribute[ind_least] -= 1;
        if (struct_distribute[ind_least] == 0) {
            close_pipe_error_checking(pipe_fd[ind_least][0], READ);
        }
    }
    // free and close file
    free(marge_arr);
    if (fclose(outfp)) {
        perror("fclose");
        exit(1);
    }
}
