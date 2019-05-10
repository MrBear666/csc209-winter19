#include <stdio.h>
#include <stdlib.h>

/* print the characters of the given array following 
 * by a single newline character.
*/
void print_state(char *arr, int size) {
    for (int i = 0; i < size; i ++) {
        printf("%c", arr[i]);
    }
    printf("\n");
}

/* update the state of the array according to the following rules:
 * the first and last elements in the array never change
 * an element whose immediate neighbours are either both "." or both "X" will become "."
 * an element whose immediate neighbours are a combination of one "." and one "X" will become "X"
 */
void update_state(char *arr, int size) {
    char prev = arr[0];
    for (int i = 1; i < (size -1); i ++) {
        if (prev == arr[i +1]) {
            prev = arr[i];
            arr[i] = '.';
        }
        else if (prev != arr[i +1]) {
            prev = arr[i];
            arr[i] = 'X';
        }
    }
}