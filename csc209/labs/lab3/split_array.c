#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/
int **split_array(const int *s, int length) {
    
    int **result = malloc(2 * sizeof(int *));
    
    if (length % 2 == 0) {
        result[0] = malloc((length/2) * sizeof(int));
        result[1] = malloc((length/2) * sizeof(int));
    }
    else {
        result[0] = malloc((length/2 + 1) * sizeof(int));
        result[1] = malloc((length/2) * sizeof(int));
    }
    
    for (int i = 0; i < length; i ++) {
        if (i % 2 == 0) {
            result[0][i/2] = s[i];
                    }
        else {
            result[1][i/2] = s[i];
	}
    }

    return result;
}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
    
    int *arr = malloc((size - 1) * sizeof(int));
    int arr_i = 0;
    for (int strs_i = 1; strs_i < size; strs_i ++) {
        arr[arr_i] = strtol(strs[strs_i], NULL, 10);
        arr_i ++;
    }
    return arr;
}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */
    int *full_array = build_array(argv, argc);
    int **result = split_array(full_array, argc -1);

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc/2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1)/2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}
