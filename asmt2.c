/* Program to generate text based on the context provided by input prompts.

  Skeleton program written by Artem Polyvyanyy, http://polyvyanyy.com/,
  September 2023, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.
  All included code is (c) Copyright University of Melbourne, 2023.

  Student Authorship Declaration:

  (1) I certify that except for the code provided in the initial skeleton file,
  the program contained in this submission is completely my own individual
  work, except where explicitly noted by further comments that provide details
  otherwise. I understand that work that has been developed by another student,
  or by me in collaboration with other students, or by non-students as a result
  of request, solicitation, or payment, may not be submitted for assessment in
  this subject. I understand that submitting for assessment work developed by
  or in collaboration with other students or non-students constitutes Academic
  Misconduct, and may be penalized by mark deductions, or by other penalties
  determined via the University of Melbourne Academic Honesty Policy, as
  described at https://academicintegrity.unimelb.edu.au.

  (2) I also certify that I have not provided a copy of this work in either
  softcopy or hardcopy or any other form to any other student, and nor will I
  do so until after the marks are released. I understand that providing my work
  to other students, regardless of my intention or any undertakings made to me
  by that other student, is also Academic Misconduct.

  (3) I further understand that providing a copy of the assignment specification
  to any form of code authoring or assignment tutoring service, or drawing the
  attention of others to such services and code that may have been made
  available via such a service, may be regarded as Student General Misconduct
  (interfering with the teaching activities of the University and/or inciting
  others to commit Academic Misconduct). I understand that an allegation of
  Student General Misconduct may arise regardless of whether or not I personally
  make use of such solutions or sought benefit from such actions.

  Signed by: [Liam Zefu Li 1463868]
  Dated:     [22/09/2023]
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* #DEFINE'S -----------------------------------------------------------------*/
#define SDELIM "==STAGE %d============================\n"   // stage delimiter
#define MDELIM "-------------------------------------\n"    // delimiter of -'s
#define THEEND "==THE END============================\n"    // end message
#define NOSFMT "Number of statements: %d\n"                 // no. of statements
#define NOCFMT "Number of characters: %d\n"                 // no. of chars
#define NPSFMT "Number of states: %d\n"                     // no. of states
#define TFQFMT "Total frequency: %d\n"                      // total frequency

#define CRTRNC '\r'                             // carriage return character
#define MAXCHARS 1000  // maximum characters per word
#define ELLIPSE "." // ellipse used in stage 1
#define ELLIPSES "..." // ellipses used in stage 1
#define TRUNCATE 37 // number of characters to be truncated in stage 1

/* Debugging area */
#define DEBUG 1
#if DEBUG
#define DUMP_DBL(x) printf("line %d: %s = %.5f\n", __LINE__, #x, x)
#define DUMP_INT(x) printf("line %d: %s = %d\n", __LINE__, #x, x)
#define DUMP_STR(x) printf("line %d: %s = %s\n", __LINE__, #x, x)
#define DUMP_CHR(x) printf("line %d: %s = %c\n", __LINE__, #x, x)
#else
#define DUMP_DBL(x)
#define DUMP_INT(x)
#define DUMP_STR(x)
#define DUMP_CHR(x)
#endif

/* TYPE DEFINITIONS ----------------------------------------------------------*/
typedef struct state state_t;   // a state in an automaton
typedef struct node  node_t;    // a node in a linked list
typedef char word_t[MAXCHARS];  // an array to store inputs

struct node {                   // a node in a linked list of transitions has
    char*           str;        // ... a transition string
    state_t*        state;      // ... the state reached via the string, and
    node_t*         next;       // ... a link to the next node in the list.
};

typedef struct {                // a linked list consists of
    node_t*         head;       // ... a pointer to the first node and
    node_t*         tail;       // ... a pointer to the last node in the list.
} list_t;

struct state {                  // a state in an automaton is characterized by
    unsigned int    id;         // ... an identifier,
    unsigned int    freq;       // ... frequency of traversal,
    int             visited;    // ... visited status flag, and
    list_t*         outputs;    // ... a list of output states.
};

typedef struct {                // an automaton consists of
    state_t*        ini;        // ... the initial state, and
    unsigned int    nid;        // ... the identifier of the next new state.
} automaton_t;


/* Function prototypes -------------------------------------------------------*/
int mygetchar(void);            // getchar() that skips carriage returns
int read_input(word_t one_line);
automaton_t *init_automaton(void);
node_t *insert_node(state_t *current_state, char ch);
void insert_state(automaton_t *automaton, node_t *node);
void initialise(word_t one_line);
void insert_statement(automaton_t *automaton, char **statement, int lineno);
void print_stage0_info(int lineno, int total_char, automaton_t *automaton);
unsigned int state_num(automaton_t *automaton);
node_t *find_node(state_t *state, char ch);
void process_prompt(automaton_t *automaton, char *prompt);
node_t *next_most_freq_node(state_t *state);
void print_truncated_part(int index, char ch);
char **read_stage0_lines(int *lineno, int *total_char, word_t one_line);
int read_num_compression(void);
void print_stage2_info(automaton_t *automaton);
int count_total_freq(automaton_t *automaton);
void do_compression(automaton_t *automaton, int num_compression);
int can_compress(state_t *current_state);
node_t *get_ascii_smaller_node(state_t *current_state);
state_t *get_compressible_state(state_t *current_state);
void create_stage2_node(state_t *current_state, state_t *next_state);
void delete_node(node_t *node);
void delete_state(state_t *state);
void free_outputs(list_t *list);

/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[]) {
    /* Stage 0 */
    word_t one_line;
    int lineno, total_char;
    // 1.read statements from the input
    char **all_lines = read_stage0_lines(&lineno, &total_char, one_line);
    // 2. set up the start state and then insert states and nodes
    automaton_t *automaton = init_automaton();
    insert_statement(automaton, all_lines, lineno);
    // 3.print the information
    printf(SDELIM, 0);
    print_stage0_info(lineno, total_char, automaton);

    /* Stage 1 */
    printf(SDELIM, 1);
    while (read_input(one_line)) {
        process_prompt(automaton, one_line);
    }

    /* Stage 2 */
    // 1. read the number of compression steps
    int num_compression = read_num_compression();
    DUMP_INT(num_compression);

    // 2. do the compression
    do_compression(automaton, num_compression);


    // 3. output the results
    printf(SDELIM, 2);
    print_stage2_info(automaton);
    printf(MDELIM);

    // 4. process the prompt

 

    return EXIT_SUCCESS;        // algorithms are fun!!!
}

/* USEFUL FUNCTIONS ----------------------------------------------------------*/

// An improved version of getchar(); skips carriage return characters.
// NB: Adapted version of the mygetchar() function by Alistair Moffat
int mygetchar() {
    int c;
    while ((c=getchar())==CRTRNC);
    return c;
}

/* ------------------------------- STAGE 0 -----------------------------------*/
char
**read_stage0_lines(int *lineno, int *total_char, word_t one_line) {
    *lineno = 0;
    *total_char = 0;
    size_t current_size = MAXCHARS;
    char **all_lines = (char **)malloc(current_size * sizeof(*all_lines));
    assert(all_lines != NULL);
    while (read_input(one_line)) {
        if (*lineno == current_size) {
            current_size *= 2;
            all_lines = realloc(all_lines, current_size * sizeof(*all_lines));
            assert(all_lines != NULL);
        }
        all_lines[*lineno] = (char *)malloc(strlen(one_line) + 1);
        assert(all_lines[*lineno] != NULL);
        strcpy(all_lines[*lineno], one_line);
        *total_char += strlen(all_lines[*lineno]);
        (*lineno)++;
    }
    return all_lines;
}


// reading input, return false if there is no input, otherwise return true 
int
read_input(word_t one_line) {
    // reset the one_line buffer before it reads
    initialise(one_line);
    
    int i = 0, ch;
    // read input and adjust memory size if needed
    while (((ch = mygetchar()) != EOF) && (ch != '\n') && i <= MAXCHARS) {
        one_line[i++] = ch;
    }
    one_line[i] = '\0';
    // if no input or just a single new line character, return false
    if (i == 0 && ch == '\n') {
        return 0;
    }
    // input read successfully, return true
    return 1;
}

// initialise the buffer array
void
initialise(word_t one_line) {
    for (int i = 0; i < MAXCHARS; i++) {
        one_line[i] = '\0';
    }
}

automaton_t
*init_automaton(void) {
    automaton_t *automaton = (automaton_t *)malloc(sizeof(*automaton));
    assert(automaton != NULL);
    automaton->ini = (state_t *)malloc(sizeof(state_t));
    assert(automaton->ini != NULL);
    automaton->ini->outputs = (list_t *)malloc(sizeof(list_t));
    assert(automaton->ini->outputs != NULL);
    automaton->ini->outputs->head = NULL;
    automaton->ini->outputs->tail = NULL;
    automaton->nid = 1;
    return automaton;
}


// insert node at foot
node_t 
*insert_node(state_t *current_state, char ch) {
    node_t *new_node = current_state->outputs->head;
    while (new_node != NULL && new_node->str[0] != ch) {
        new_node = new_node->next;
    }

    if (new_node == NULL) {
        // allocate memory for each component
        new_node = (node_t*)malloc(sizeof(node_t));
        assert(new_node != NULL);
        new_node->str = (char*)malloc(1+sizeof(char));
        assert(new_node->str != NULL);
        
        // initialise everything
        new_node->str[0] = ch;
        new_node->str[1] = '\0';
        new_node->state = NULL; 
        new_node->next = NULL;

        // Add the node to the current state's outputs
        if (current_state->outputs->head == NULL) {
            current_state->outputs->head = new_node;
            current_state->outputs->tail = new_node;
        } else {
            current_state->outputs->tail->next = new_node;
            current_state->outputs->tail = new_node;
        }        
    }
    return new_node;
}

// insert at foot
void
insert_state(automaton_t *automaton, node_t *node) {
    assert(automaton != NULL);
    // set up id and frequency
    if (node->state == NULL) {
        state_t *new_state = (state_t *)malloc(sizeof(*new_state));
        assert(new_state != NULL);
        new_state->outputs = (list_t *)malloc(sizeof(list_t));
        assert(new_state->outputs != NULL);
        node->state = new_state;
        new_state->id = automaton->nid++;
        new_state->freq = 1; // minus number of leaf states at the end 
        new_state->visited = 0;
        new_state->outputs->head = new_state->outputs->tail = NULL;
    } else {
        node->state->freq++;
    }
}

// insert a statement into the automaton
void
insert_statement(automaton_t *automaton, char **statement, int lineno) {
    for (int i = 0; i < lineno; i++) {
        state_t *current_state = automaton->ini;
        for (int j = 0; statement[i][j] != '\0'; j++) {
            node_t *node = insert_node(current_state, statement[i][j]);
            insert_state(automaton, node);
            current_state = node->state;
        }
    }
}

// print the stage0 informations that are required
void
print_stage0_info(int lineno, int total_char, automaton_t *automaton) {
    printf(NOSFMT, lineno);
    printf(NOCFMT, total_char);
    printf(NPSFMT, state_num(automaton));
}

// count the total numbers of states in the automaton
unsigned int state_num(automaton_t *automaton) {
    return automaton->nid;
}

/* ------------------------------- STAGE 1 -----------------------------------*/

// Helper function of stage 1, print the part that needs to be truncated
void
print_truncated_part(int index, char ch) {
    printf("%c", ch);
    if (index + strlen(ELLIPSES) < TRUNCATE) {
        printf(ELLIPSES);
    } else {
        // go to next index to drop one ellipse
        index++;
        while (index < TRUNCATE) {
            printf(ELLIPSE);
            index++;
        }
    }
    printf("\n");
}

void
process_prompt(automaton_t *automaton, char *prompt) {
    // 1. replay the prompt
    state_t *current_state = automaton->ini;
    int i;
    for (i = 0; prompt[i] != '\0'; i++) {
        node_t *next_node = find_node(current_state, prompt[i]);
        // if a character is not in the automaton, exit the replay
        if (next_node == NULL) {
            print_truncated_part(i, prompt[i]);
            return;
        }
        // if the output reach the truncate limit, exit the replay
        if (i >= TRUNCATE) {
            printf("\n");
            return;
        }
        printf("%c", prompt[i]);
        current_state->freq++;
        current_state = next_node->state;
    }
    
    // 2. print ellipses 
    printf(ELLIPSES);
    i += strlen(ELLIPSES);

    // 3. do the trace of the automaton to generate text
    while (current_state->outputs->head != NULL && i < TRUNCATE) {
        node_t *next_node = next_most_freq_node(current_state);
        printf("%c", next_node->str[0]);
        current_state = next_node->state;
        i++;
    }
    printf("\n");
}

// find the correct node given the input characters
node_t
*find_node(state_t *state, char ch) {
    node_t *current_node = state->outputs->head;
    while (current_node != NULL) {
        // this is the right node, found it!
        if (current_node->str[0] == ch) {
            return current_node;
        }
        // not this one, go next
        current_node = current_node->next;
    }
    // process through all the nodes, nothing found
    return NULL;
}

// return node on the walk until leaf state by the most frequent one, if same 
// frequency, then get the one that is ASCIIbetically greater
node_t
*next_most_freq_node(state_t *state) {
    node_t *current_node = state->outputs->head;
    node_t *most_freq_node = current_node;
    while (current_node != NULL) {
        // compare frequency
        if (current_node->state->freq > most_freq_node->state->freq) {
            most_freq_node = current_node;
        } 
        // frequency the same, compare their ASCII character
        if (current_node->state->freq == most_freq_node->state->freq 
            && current_node->str[0] > most_freq_node->str[0]) {
            most_freq_node = current_node;
        }
        current_node = current_node->next;
    }
    return most_freq_node;
}

/* ------------------------------- STAGE 2 -----------------------------------*/

// reads the number of compression needed
int
read_num_compression(void) {
    int num_compression;
    scanf("%d", &num_compression);
    return num_compression;
}

// check if the statement is compressible
int
can_compress(state_t *current_state) {
    // if current state does not have only 1 outgoing node, return false
    if (current_state->outputs->head != current_state->outputs->tail) {
        return 0;
    }
    state_t *next_state = current_state->outputs->head->state;
    // if next state does not have one or more outgoing node, return false
    if (next_state->outputs->head == NULL) {
        return 0;
    }
    // all test passed, return true
    return 1;
}

// return the ASCIIbetically smaller node to be first compressed
node_t
*get_ascii_smaller_node(state_t *current_state) {  
    node_t *node = current_state->outputs->head;
    node_t *smallest_node = node;
    while (node != NULL) {
        if (strcmp(smallest_node->str, node->str) > 0) {
            smallest_node = node;
        }
        node = node->next;
    }
    return smallest_node;
}

state_t
*get_compressible_state(state_t *current_state) {
    // base case 1, if reach the leaf state or is visited, return NULL
    if (current_state == NULL || current_state->visited == 1) {
        return NULL;
    }

    // now its visited
    current_state->visited = 1;

    // base case 2, if the current state is compressible, return state
    if (can_compress(current_state)) {
        return current_state;
    }

    // if current state not compressible, depth first search in outgoing states
    node_t *node = get_ascii_smaller_node(current_state);
    while (node != NULL) {
        state_t *compress_state = get_compressible_state(node->state);
        if (compress_state != NULL) {
            return compress_state;
        }
        node = node->next;
    }
    // search done, no state is compressible from this state
    return NULL;
}

void
create_stage2_node(state_t *current_state, state_t *next_state) {
    node_t *compress_node = current_state->outputs->head; // single output
    node_t *outgoing_node = next_state->outputs->head; // one or more outputs
    int first_node_created = 0; // flag to indicate if first node is created
    while (outgoing_node != NULL) {
        int total_str_len;
        total_str_len = strlen(compress_node->str) + strlen(outgoing_node->str);

        // allocate memory
        node_t *new_node = (node_t *)malloc(sizeof(node_t));
        assert(new_node != NULL);
        new_node->str = (char *)malloc(total_str_len + 1);
        assert(new_node->str != NULL);

        // copy the string into the new node
        strcpy(new_node->str, compress_node->str);
        strcat(new_node->str, outgoing_node->str);

        // adjust pointers
        new_node->state = outgoing_node->state;
        new_node->next = NULL;
        if (!first_node_created) {
            current_state->outputs->head = new_node;
            current_state->outputs->tail = new_node;
            first_node_created = 1;
        } else {
            current_state->outputs->tail->next = new_node;
            current_state->outputs->tail = new_node;
        }
        // goes to the next node and continue to create new nodes for stage 2
        outgoing_node = outgoing_node->next;
    }
}

// perform the compression in stage 2
void
do_compression(automaton_t *automaton, int num_compression) {
    for (int i = 0; i < num_compression; i++) {
        // get the compressible state first
        DUMP_INT(i);
        state_t *current_state = get_compressible_state(automaton->ini);
        if (current_state == NULL) {
            break;
        }
        state_t *next_state = current_state->outputs->head->state;
        node_t *old_node = current_state->outputs->head;
        // create new nodes for the current state
        create_stage2_node(current_state, next_state);
        
        // delete the original nodes and state
        delete_node(old_node);
        delete_state(next_state);

        // decrement the number of states
        automaton->nid--;
    }
}

// delete a node and free the memory used for it
void
delete_node(node_t *node) {
    assert(node != NULL && node->str != NULL && node->state != NULL);
    free(node->str);
    node->str = NULL;
    free(node->state);
    node->state = NULL;
    free(node);
}

// delete a state and free the memory used for it
void
delete_state(state_t *state) {
    assert(state != NULL);
    free_outputs(state->outputs);
    free(state);
}

// free memory used by output list
void
free_outputs(list_t *list) {
    assert(list != NULL);
    node_t *current_node, *previous_node;
    current_node = list->head;
    while (current_node != NULL) {
        previous_node = current_node;
        current_node = current_node->next;
        delete_node(previous_node);
    }
    free(list);
}

// print the information of stage 2
void 
print_stage2_info(automaton_t *automaton) {
    printf(NPSFMT, state_num(automaton));
    printf(TFQFMT, count_total_freq(automaton));
}

// counts the total frequency in the automaton
int
count_total_freq(automaton_t *automaton) {
    // recursively count the frequency
    int total_freq = 0;
    // ...
    return total_freq;
}



/* THE END -------------------------------------------------------------------*/

/* count the total numbers of states in the automaton
unsigned int 
state_num(state_t *state) {
    // base case, if reach the leaf state or is visited, end recursion
    if (state == NULL || state->visited == 1) {
        return 0;
    }
    state->visited = 1;
    unsigned int count = 1;
    node_t* node = state->outputs->head;
    while (node != NULL) {
        count += state_num(node->state);
        node = node->next;
    }
    return count;
}
*/