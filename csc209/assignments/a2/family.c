#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;


/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}


/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family* fam_list) {
    int i;
    Family *fam = fam_list;
    
    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for(i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is 
   a copy of str. Initialize word_ptrs to point to 
   family_increment+1 pointers, numwords to 0, 
   maxwords to family_increment, and next  to NULL.
*/
Family *new_family(char *str) {
    Family *family = malloc(sizeof(Family));
    if (family == NULL) { //check err
        perror("malloc");
        exit(1);
    }
    family->signature = malloc(sizeof(char) * (strlen(str) + 1));
    if (family->signature == NULL) { //check err
        perror("malloc");
        exit(1);
    }

    strcpy(family->signature, str);
    family->word_ptrs = malloc((family_increment + 1)* sizeof(char *));
    if (family->word_ptrs == NULL) { //check err
        perror("malloc");
        exit(1);
    }

    family->num_words = 0;
    family->max_words = family_increment;
    family->next = NULL;

    return family;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
    // if fam->word_ptrs full
    if (fam->num_words == fam->max_words) {
        fam->word_ptrs = realloc(fam->word_ptrs, (family_increment + fam->max_words + 1) * (sizeof(char *)));
        if (fam->word_ptrs == NULL) { //check err
        perror("realloc");
        exit(1);
        }
        fam->max_words = family_increment + fam->max_words;
    }
    fam->word_ptrs[fam->num_words] = word;
    fam->num_words += 1;
    fam->word_ptrs[fam->num_words] = NULL;
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
    Family *fam = fam_list;
    while (fam) {
        if (strcmp(sig, fam->signature) == 0) {
            return fam;
        }
        fam = fam->next;
    }
    return NULL;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
    int largest = 0;
    Family *largest_family = NULL;
    Family *fam = fam_list;

    while (fam) {
        // if fam->num_words > largest，update largest and the largest_family
        if (fam->num_words > largest) {
            largest = fam->num_words;
            largest_family = fam;
        }
        fam = fam->next;
    }
    return largest_family;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
    Family *fam = fam_list;
    while (fam != NULL) {
        free(fam->word_ptrs);
        free(fam->signature);
        Family *pre_fam = fam;
        fam = fam->next;
        free(pre_fam);
    }
}


/* Create a string signature of the word by according to letter,
   and return the signature.
*/
char *create_sig_of_word(char *word, char letter) {
    int i = 0;
    int len = strlen(word);
    char *signature = malloc(sizeof(char) * (len + 1));
    if (signature == NULL) { //check err
        perror("malloc");
        exit(1);
    }

    while (word[i] != '\0') {
        // if match the letter then build as the letter,
        // otherwise as '-'
        if (word[i] == letter) {
            signature[i] = letter;
        }
        else {
            signature[i] = '-';
        }
        i ++;
    }
    signature[len] = '\0';
    return signature;
}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
    char **word = word_list;
    Family *fam_list = NULL;
    Family *fam_with_sig = NULL;
    char *sig = NULL;

    while (*word) {
        sig = create_sig_of_word(*word, letter);
        // find the family with signature sig 
        fam_with_sig = find_family(fam_list, sig);

        // if no such family with signature sig 
        if (fam_with_sig == NULL) {
            Family *new_fam = new_family(sig);
            add_word_to_family(new_fam, *word);

            // link to the front of fam_list
            new_fam->next = fam_list;
            fam_list = new_fam;
        }
        else { // have such family with sig, add word directly
            add_word_to_family(fam_with_sig, *word);  
        }
        word ++;
        free(sig);
    }
    return fam_list;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    return fam->signature;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
    int size = fam->num_words + 1;
    char **new_word_list = malloc(sizeof(char *) * (size));
    if (new_word_list == NULL) { // check err
        perror("malloc");
        exit(1);
    }
    // copy word list
    for (int i = 0; i < size; i ++) {
        new_word_list[i] = fam->word_ptrs[i];
    }
    return new_word_list;
}


/* Return a pointer to a random word from fam. 
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
    int r = (rand() % fam->num_words);
    return fam->word_ptrs[r];
}
