#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "socket.h"
#include "gameplay.h"

#ifndef PORT
    #define PORT 57755
#endif
#define MAX_QUEUE 5


void add_player(struct client **top, int fd, struct in_addr addr);
void remove_player(struct client **top, int fd);

/* These are some of the function prototypes that we used in our solution 
 * You are not required to write functions that match these prototypes, but
 * you may find the helpful when thinking about operations in your program.
 */
/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf);
void announce_turn(struct game_state *game);
void announce_winner(struct game_state *game);
/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game);
int username_exist(char inbuf[], struct client **head);

void remove_active_p(struct game_state *game, int fd);
void remove_active_if_write_err(struct client **p, char *outbuf, struct game_state *game);
void remove_new_if_write_err(struct client *p, char *outbuf, struct client *new_players);
void formating_err_msg_exit(struct client *p);
void guess_wrong_msgs(struct game_state *game, char letter, struct client *p);
void no_guess_left_msg(struct game_state *game);
void letter_in_word(char **argv, struct game_state *game, char letter, struct client *p);
void letter_is_guessed(char letter, struct client *p, struct game_state *game);
void guess_is_not_letter(char letter, struct game_state *game, struct client *p);
void check_head_null(struct game_state *game);
void guess_too_many_or_empty(struct client *p, int cur_fd, struct game_state *game);
void not_cur_p_typed(int rbytes, struct client *p, struct game_state *game);
void handle_new_player(struct client **new_players, struct client *p, struct game_state *game);
void join_message(struct client *p, struct game_state *game);
void join_status(struct game_state *game, struct client *p);
void name_exited(struct client *p, struct client *new_players);
void input_name_is_empty(struct client *p, struct client *new_players);

/* The set of socket descriptors for select to monitor.
 * This is a global variable because we need to remove socket descriptors
 * from allset when a write to a socket fails.
 */
fd_set allset;

/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset 
 */
void remove_player(struct client **top, int fd) {
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Disconnect from %s\n", inet_ntoa((*p)->ipaddr));
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}

/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf) {
    struct client **p;
    for (p = &(game->head); *p; p = &(*p)->next) {
        remove_active_if_write_err(p, outbuf, game);
    }
}

/* announce who is the current player in the game, the announcement is 
 * different to the current player and all other players.
 */
void announce_turn(struct game_state *game) {
    struct client **p;
    char turn_msg[MAX_MSG + 1];

    for (p = &(game->head); *p; p = &(*p)->next) {
        if ((*p)->fd == (game->has_next_turn)->fd) {
            sprintf(turn_msg, "Your guess?\r\n");
            remove_active_if_write_err(p, turn_msg, game);
        } 
        else {
            sprintf(turn_msg, "It is %s's turn.\r\n", (game->has_next_turn)->name);
            remove_active_if_write_err(p, turn_msg, game);
        }
    }
}
/* announce who is the winner in the game, the announcement is 
 * different to the winner and all other players.
 */
void announce_winner(struct game_state *game) {
    struct client **p;
    char turn_msg[MAX_MSG  + 1];

    for (p = &(game->head); *p; p = &(*p)->next) {
        if ((*p)->fd == (game->has_next_turn)->fd) {
            sprintf(turn_msg, \
                    "The word was %s.\r\nGame over! You win!\r\n\r\n\r\nLet's start a new game:\r\n", \
                    game->word);
            remove_active_if_write_err(p, turn_msg, game);
        } 
        else {
            sprintf(turn_msg, "The word was %s.\r\nGame over! %s won!\r\n\r\n\r\nLet's start a new game:\r\n", \
                                                                game->word, (game->has_next_turn)->name);
            remove_active_if_write_err(p, turn_msg, game);
        }
    }
}

/* Move the has_next_turn pointer to the next active client if there is one,
 * otherwise moves to the head.
 */
void advance_turn(struct game_state *game) {
    if (game->has_next_turn != NULL && game->has_next_turn->next == NULL){
        game->has_next_turn = game->head;
    }
    else {
        game->has_next_turn = game->has_next_turn->next;
    }
}

/* chech username is exist or not. Return 0 if not exist, 1 otherwise.
 */
int username_exist(char inbuf[], struct client **head) {
    struct client **h;
    for (h = head; *h; h = &(*h)->next) {
        if (strcmp((*h)->name, inbuf) == 0) {
            return 1;
        }
    }
    return 0;
}

/* remove function for the active players. */ 
void remove_active_p(struct game_state *game, int fd) {
    if ((game->has_next_turn)->fd == fd) {
        // if we remove the current player, we need to advance turn.
        advance_turn(game);
    }
    remove_player(&(game->head), fd);
}

/* check write error and if write return -1, remove the player from active list
 */
void remove_active_if_write_err(struct client **p, char *outbuf, struct game_state *game) {
    if(write((*p)->fd, outbuf, strlen(outbuf)) == -1) {
        fprintf(stderr, "Write to client %s failed\n", inet_ntoa((*p)->ipaddr));
        remove_active_p(game, (*p)->fd);
    }
}

/* check write error and if write return -1, remove the player from new-players list
 */
void remove_new_if_write_err(struct client *p, char *outbuf, struct client *new_players) {
    if(write(p->fd, outbuf, strlen(outbuf)) == -1) {
        fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
        remove_player(&new_players, p->fd);
    }
}

/* formating the error out message when read return -1
 */
void formating_err_msg_exit(struct client *p) {
    char remove_msg[MAX_MSG + 1];
    sprintf(remove_msg, "Read from client %s failed\n", inet_ntoa(p->ipaddr));
    perror(remove_msg);
    exit(1);
}

/* write guess wrong msg to player and check write err
 */
void guess_wrong_msgs(struct game_state *game, char letter, struct client *p) {

    printf("BAD guess. Letter %c is not in the word\n", letter);
    char guess_wrong_msg[MAX_MSG + 1];
    sprintf(guess_wrong_msg, "%c is not in the word.\r\n", letter);
    remove_active_if_write_err(&p, guess_wrong_msg, game);
}

/* formating msg and broadcast msg, when no guess remaining in the cur game.
 */
void no_guess_left_msg(struct game_state *game) {
    char no_guess_msg[MAX_MSG + 1];
    sprintf(no_guess_msg, "No more guesses. The word was %s.\r\n\r\nLet's start a new game:\r\n", game->word);
    printf("Evaluating for game_over\n");
    broadcast(game, no_guess_msg);
}

/* handle teh case when guess-letter is not being gusee and the letter matched word.
 * also, handle the win-case if we have a winner
 */
void letter_in_word(char **argv, struct game_state *game, char letter, struct client *p) {
    printf("GOOD guess. Letter %c is in the word\n", letter);
    for(int i = 0; i < strlen(game->word); i++) {
        if ((game->word)[i] == letter) {
            (game->guess)[i] = letter;
        }
    }
    char letter_guess_msg[MAX_MSG + 1];
    sprintf(letter_guess_msg, "%s guesses: %c\r\n", p->name, letter);
    broadcast(game, letter_guess_msg);
    // check if win
    if (strchr(game->guess, '-') == NULL) {
        announce_winner(game);
        printf("Game over. %s won!\n", p->name);
        init_game(game, argv[1]);
        printf("New Game\n");
    }
}

/* handle tha case when guess-letter is being guessed, write to the player and 
 * tell him/her he/she guessed a letter that is already being guessed
 */
void letter_is_guessed(char letter, struct client *p, struct game_state *game) {
    printf("INVALID guess. Letter %c is already being guessed\n", letter);
    char re_guess_msg[MAX_MSG + 1];
    sprintf(re_guess_msg, "%c is already being guessed. Please guess a new letter.\r\n", letter);
    remove_active_if_write_err(&p, re_guess_msg, game);
}

/* handle tha case when guess-letter is not a letter, write to the player and 
 * tell him/her he/she need to guess a letter.
 */
void guess_is_not_letter(char letter, struct game_state *game, struct client *p) {
    printf("INVALID guess. Character %c is not a letter\n", letter);
    // not alphabet
    char *re_enter_guess = "Not a valid guess. Please enter a guess letter again:\r\n";
    remove_active_if_write_err(&p, re_enter_guess, game);
}

/* check if no player in game.head, we also need to set the game.has_next_turn to NULL
 */
void check_head_null(struct game_state *game) {
    if (game->head == NULL) {
        game->has_next_turn = NULL;
    }
    else { //server
        printf("It's %s's turn.\n", game->has_next_turn->name);
    }
}

/* handle tha case when guess-letter is a string that more than 2 bit or empty string.
 * write to the player and tell him/her he/she needs to guess a letter.
 */
void guess_too_many_or_empty(struct client *p, int cur_fd, struct game_state *game) {
    (p->in_ptr - 2)[0] = '\0';
    printf("[%d] Found newline %s\n", cur_fd, p->inbuf);
    printf("INVALID guess. String %s is not a letter\n", p->inbuf);

    p->in_ptr = p->inbuf;
    char *re_enter_guess = "Not a valid guess. Please enter a guess letter again:\r\n";
    remove_active_if_write_err(&p, re_enter_guess, game);
} 

/* handle tha case when non-current player types something and send it to server,
 * we need to announce hin/her that he/she is not on turn.
 */
void not_cur_p_typed(int rbytes, struct client *p, struct game_state *game) {
    p->in_ptr += rbytes; 
    if ((p->in_ptr - p->inbuf >= 2) && (p->in_ptr - 1)[0] == '\n' && (p->in_ptr - 2)[0] == '\r') {
        (p->in_ptr - 2)[0] = '\0';
        printf("[%d] Found newline %s\n", p->fd, p->inbuf);
        printf("Player %s tried to guess out of turn.\n", p->name);
        char *not_ur_turn_msg = "It is not your turn to guess.\r\n";
        remove_active_if_write_err(&p, not_ur_turn_msg, game);
        p->in_ptr = p->inbuf;
    }
}

/* handle new player, remove it from new-player list and add he/she to the front of game.head
 * also, set game->has_next_turn if needed.
 */
void handle_new_player(struct client **new_players, struct client *p, struct game_state *game) {
    struct client **p2;
    
    for (p2 = new_players; *p2 && (*p2)->fd != p->fd; p2 = &(*p2)->next)
        ;
    if (*p2) {
        struct client *t = (*p2)->next;
        *p2 = t;
    }
    else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", p->fd);
    }
    // add to the front of active player list and set has_next_turn if needed.
    p->next = game->head;
    game->head = p;
    if (game->has_next_turn == NULL) {
        game->has_next_turn = p;
    }
}  

/* formating the join msg and broadcast msg.
 */
void join_message(struct client *p, struct game_state *game) {

    printf("[%d] Found newline %s\n", p->fd, p->name);
    printf("%s has just joined.\n", p->name);

    char join_msg_in[MAX_MSG + 1];
    sprintf(join_msg_in, "%s has just joined.\r\n", p->name);                  
    printf("It's %s's turn.\n", game->has_next_turn->name);
    broadcast(game, join_msg_in);
}

/* formating the join status of game after new player add into active list,
 * and announce status to him/her
 */
void join_status(struct game_state *game, struct client *p){
    char status_msg[MAX_MSG];
    status_message(status_msg, game);
    remove_active_if_write_err(&p, status_msg, game);
}

/* handle the case that player chosen an existing name and need to re-enter a name
 */
void name_exited(struct client *p, struct client *new_players) {
    p->in_ptr = p->inbuf;
    printf("[%d] Found newline %s\n", p->fd, p->inbuf);
    printf("Client %d chosen an existing name, %s.\n", p->fd, p->inbuf);
    // ask to re-enter a username
    char *re_enter_name = "Name already exist. Please enter a new username:\r\n";
    remove_new_if_write_err(p, re_enter_name, new_players);
}

/* handle that case when input-username is an empty string
 */
void input_name_is_empty(struct client *p, struct client *new_players) {
    p->in_ptr = p->inbuf;
    printf("Client %d chosen an empty username\n", p->fd);
    char *re_enter_name = "Not an accepted name format. Please enter a username again:\r\n";
    remove_new_if_write_err(p, re_enter_name, new_players);
}

/* play the word guess game through internet by using socket and so ons
 */
int main(int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;
    
    if(argc != 2){
        fprintf(stderr,"Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int)time(NULL));
    // Set up the file pointer outside of init_game because we want to 
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);
    
    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;
    
    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages.  In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;
    
    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);
    
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if(write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&new_players, clientfd);
            };
        }
        
        /* Check which other socket descriptors have something ready to read.
         * The reason we iterate over the rset descriptors at the top level and
         * search through the two lists of clients each time is that it is
         * possible that a client will be removed in the middle of one of the
         * operations. This is also why we call break after handling the input.
         * If a client has been removed the loop variables may not longer be 
         * valid.
         */
        int cur_fd;
        for(cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if(FD_ISSET(cur_fd, &rset)) {
                // Check if this socket descriptor is an active player
                
                for(p = game.head; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        //TODO - handle input from an active client
                        // handle current player is guessing or typing input
                        if (cur_fd == (game.has_next_turn)->fd) {
                            int rbytes = read(p->fd, p->in_ptr, MAX_WORD);
                            if (rbytes == -1) { 
                                // formate err msg and exit with msg
                                formating_err_msg_exit(p);
                            }
                            // to server
                            printf("[%d] Read %d bytes\n", cur_fd, rbytes);
                            if (rbytes == 0) {
                                // keep goodbye-player's name for usage of broadcast
                                char goodbye_msg[MAX_MSG + 1];
                                sprintf(goodbye_msg, "Goodbye %s\r\n", p->name);
                                remove_active_p(&game, p->fd);
                                broadcast(&game, goodbye_msg);

                                // check game.head NULL(no player in active-list after removing) 
                                check_head_null(&game);
                                announce_turn(&game); 
                                break;
                            }
                            p->in_ptr += rbytes;
                            // guess one character (not sure is alphabet or not)
                            if ((p->in_ptr - p->inbuf == 3) && (p->in_ptr - 2)[0] == '\r' && \
                                                                (p->in_ptr - 1)[0] == '\n') {
                                // check guess is alphabet
                                char letter = p->inbuf[0];
                                p->in_ptr = p->inbuf;
                                printf("[%d] Found newline %c\n", cur_fd, letter);
                                
                                // guess letter is alphbet
                                if ('a' <= letter && letter <= 'z') {
                                    // letter is not being guessed
                                    if (game.letters_guessed[letter - 'a'] == 0) {

                                        // set letter index to 1, 1 means it has being guessed
                                        game.letters_guessed[letter - 'a'] = 1; 
                                        char *chr = strchr(game.word, letter);

                                        // letter not in the word
                                        if (chr == NULL) {
                                            game.guesses_left -= 1;
                                            // turn change 
                                            advance_turn(&game);
                                            // write guess wrong msg to player and check write err
                                            guess_wrong_msgs(&game, letter, p);
                                            char letter_guess_msg[MAX_MSG + 1];
                                            sprintf(letter_guess_msg, "%s guesses: %c\r\n", p->name, letter);
                                            broadcast(&game, letter_guess_msg);
                                            // if game over(no guess remaining left)
                                            if (game.guesses_left == 0) {
                                                no_guess_left_msg(&game);
                                                printf("New Game\n");
                                                init_game(&game, argv[1]);
                                            }
                                        }
                                        // if letter not being gusee and letter in the word
                                        else {
                                            letter_in_word(argv, &game, letter, p);
                                        }
                                        // show game status and announcement
                                        char status_msg[MAX_MSG];
                                        status_message(status_msg, &game);
                                        broadcast(&game, status_msg);
                                        printf("It's %s's turn.\n", game.has_next_turn->name);
                                        announce_turn(&game);
                                    }
                                    else {
                                        // letter is being guessed. re-guess
                                        letter_is_guessed(letter, p, &game);
                                    }
                                }
                                else {
                                    // guess is not letter
                                    guess_is_not_letter(letter, &game, p);
                                }
                            }
                            else if ((p->in_ptr - p->inbuf != 3) && (p->in_ptr - 2)[0] == '\r' && \
                                                                    (p->in_ptr - 1)[0] == '\n') {
                                //guess not one bit
                                guess_too_many_or_empty(p, cur_fd, &game);
                            }
                        }
                        else {
                            //not cur player, should not input anything
                            int rbytes = read(p->fd, p->in_ptr, MAX_BUF);
                            if (rbytes == -1) {
                                formating_err_msg_exit(p);
                            }
                            printf("[%d] Read %d bytes\n", cur_fd, rbytes);
                            if (rbytes == 0) {
                                //keep goodbye-player's name
                                char goodbye_msg[MAX_MSG + 1];
                                sprintf(goodbye_msg, "Goodbye %s\r\n", p->name);
                                remove_active_p(&game, p->fd);
                                // goodbye 
                                broadcast(&game, goodbye_msg);
                                announce_turn(&game); 
                            }
                            else if (rbytes > 0) {
                                // not cur player type something
                                not_cur_p_typed(rbytes, p, &game);
                            }
                        }
                        break;
                    }
                }
                // Check if any new players are entering their names
                for(p = new_players; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        // TODO - handle input from an new client who has
                        // gather username
                        int rbytes = read(p->fd, p->in_ptr, MAX_NAME);
                        if (rbytes == -1) {
                            formating_err_msg_exit(p);
                        }
                        printf("[%d] Read %d bytes\n", cur_fd, rbytes);
                        // if disconnection
                        if (rbytes == 0) {
                            remove_player(&new_players, p->fd);
                            break;
                        }
                        // if read \r\n and have an name                        
                        p->in_ptr += rbytes;
                        if ((p->in_ptr - p->inbuf > 2) && (p->in_ptr - 2)[0] == '\r' && \
                                                            (p->in_ptr - 1)[0] == '\n') {
                            (p->in_ptr - 2)[0] = '\0';
                            // username not exist, 0 means not esist name
                            if (username_exist(p->inbuf, &game.head) == 0) {
                                strcat(p->name, p->inbuf);
                                p->in_ptr = p->inbuf;
                                // remove play from new_players and add to the 
                                // front of active players list(game.head)
                                handle_new_player(&new_players, p, &game);
                                join_message(p, &game);
                                // display the game status for player only
                                join_status(&game, p);
                                announce_turn(&game);
                            }
                            else {
                                // player chosen an existing name and need to re-enter a name
                                name_exited(p, new_players);
                            }
                        }
                        else if ((p->in_ptr - p->inbuf == 2) && (p->in_ptr - 2)[0] == '\r' && \
                                                                (p->in_ptr - 1)[0] == '\n') { 
                            // username is empty string
                            input_name_is_empty(p, new_players);
                        }  
                        break;
                    } 
                }
            }
        }
    }
    return 0;
}


