#ifndef FAMILY_H
#define FAMILY_H

struct fam {
    char *signature; /* Family signature; e.g. ---e */
    char **word_ptrs; /* Words belonging to family */
    int num_words; /* Number of words in family */
    int max_words; /* Number of total pointers in word_ptrs so far */
    struct fam *next; /* NULL means end of list */
};
typedef struct fam Family; 


void init_family(int size);
void print_families(Family* fam_list);
Family *new_family(char *str);
void add_word_to_family(Family *fam, char *word);
Family *find_family(Family *fam_list, char *sig);
Family *find_biggest_family(Family *fam_list);
void deallocate_families(Family *fam_list);
Family *generate_families(char **word_list, char letter);
char *get_family_signature(Family *fam);
char **get_new_word_list(Family *fam);
char *get_random_word_from_family(Family *fam);

#endif
