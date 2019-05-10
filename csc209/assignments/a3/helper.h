#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44
//for error checking
#define READ 0
#define WRITE 1

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
int *divide_up_words(int num_structs, int num_processes);
void close_pipe_error_checking(int int_pipe, int type);
void read_write_error_checking(int check_num, int type, int bytes_wanted);
struct rec *struct_arr_form_file(int num, FILE *infp);
void sorted_array_to_pipe(int *struct_distribute, char *infp, int ind_child, int pipe_fd);
struct rec *set_up_marge_arr(int num_processes, int *struct_distribute, int pipe_fd[][2]);
void merge_func(int num_processes, int *struct_distribute, int pipe_fd[][2], int num_structs, char *outfp);


#endif /* _HELPER_H */
