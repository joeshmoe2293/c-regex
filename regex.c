#include "regex.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Macros
 */

#define REGEX_MALLOC(type) ((type *)malloc(sizeof(type)))

/*
 * Private structure / enum types
 */
enum regex_type_e {
    ERROR,
    REG,
    STAR,
    PLUS,
    WILD,
    ANCHOR,
    END
};

enum regex_size_e {
    SINGLE,
    GROUP
};

struct regex_t {
    enum regex_type_e type;
    enum regex_size_e size;
    int group_size;
    char to_match;
    char *group_to_match;
};

struct compiled_regex_t {
    struct regex_t regex;
    struct compiled_regex_t *next;
};

/*
 * Priavte function prototypes
 */
static int regex_match_rec(char *regex, char *string);

static int regex_match_star(char c, char *regex, char *string);

static struct compiled_regex_t * regex_compile(char *regex);

static struct compiled_regex_t * malloc_regex();

static struct compiled_regex_t * compile_atom(char *atom);

static struct compiled_regex_t * consume_atom(char *atom);

static enum regex_type_e is_repeated(char *atom);

static struct compiled_regex_t * clone(struct compiled_regex_t *node);

static struct compiled_regex_t * last_node(struct compiled_regex_t *head);

static char * next_atom(char *regex);

static int regex_is_token(char t);

static void print_compiled_regex(struct compiled_regex_t *regex);

static void free_compiled_regex(struct compiled_regex_t *regex);

static int regex_match_compiled_loop(struct compiled_regex_t *regex, char *string);

static int regex_match_compiled_star(struct compiled_regex_t *regex, char *string);

static int regex_match_compiled_group(struct compiled_regex_t *regex, char *string);

/*
 * Public function definitions
 */
int regex_match(char *regex, char *string)
{
    char *t = string;

    if (*regex == '^') {
        return regex_match_rec(regex + 1, string);
    }

    do {
        if (regex_match_rec(regex, t)) {
            return 1;
        }
    } while (*t++ != '\0');

    return 0;
}

int regex_match_compiled(char *regex, char *string)
{
    int result = 0;
    char *t = string;
    struct compiled_regex_t *compiled = regex_compile(regex);

    if (compiled->regex.type == ANCHOR) {
        result = regex_match_compiled_loop(compiled->next, string);
    } else {
        do {
            if (regex_match_compiled_loop(compiled, t)) {
                result = 1;
            }
        } while (*t++ != '\0');
    }

    free_compiled_regex(compiled);
    return result;
}

/*
 * Private function definitions
 */

int regex_match_rec(char *regex, char *string)
{
    if (*regex == '\0') {
        return 1;
    } else if (regex[1] == '*') {
        return regex_match_star(*regex, regex + 2, string);
    } else if (regex[1] == '+' && *string == *regex) {
        return regex_match_star(*regex, regex + 2, string + 1);
    } else if (*regex == '$' && regex[1] == '\0') {
        return *string == '\0';
    } else if (*regex == *string) {
        return regex_match_rec(regex + 1, string + 1);
    }

    return 0;
}

int regex_match_star(char c, char *regex, char *string)
{
    char *t = string;

    while (*t != '\0' && (*t == c || c == '.')) {
        t++;
    }

    do {
        if (regex_match_rec(regex, t)) {
            return 1;
        }
    } while (t-- > string);

    return 0;
}

struct compiled_regex_t * regex_compile(char *regex)
{
    struct compiled_regex_t *compiled = NULL;
    char *t = regex;
    
    if (*t != '\0') {
        compiled = compile_atom(t);
        last_node(compiled)->next = regex_compile(next_atom(t));
    }

    return compiled;
}

struct compiled_regex_t * malloc_regex()
{
    struct compiled_regex_t *allocated;

    allocated = REGEX_MALLOC(struct compiled_regex_t);
    allocated->regex.type = ERROR;
    allocated->regex.size = SINGLE;
    allocated->regex.group_size = 0;
    allocated->regex.to_match = '\0';
    allocated->regex.group_to_match = NULL;
    allocated->next = NULL;

    return allocated;
}

struct compiled_regex_t * compile_atom(char *atom)
{
    struct compiled_regex_t *returned_regex = NULL;
    struct compiled_regex_t *consumed = consume_atom(atom);
    enum regex_type_e repeated_type = is_repeated(atom);

    switch (repeated_type) {
        case PLUS:
            returned_regex = clone(consumed);
            returned_regex->regex.type = STAR;
            consumed->next = returned_regex;
            break;
        case STAR:
            consumed->regex.type = STAR;
        default:
            break;
    }
    
    returned_regex = consumed;

    return returned_regex;
}

struct compiled_regex_t * consume_atom(char *atom)
{
    struct compiled_regex_t *regex_node = malloc_regex();
    char *t = atom;
    int group_size = 0;

    switch (*atom) {
        case '^':
            regex_node->regex.type = ANCHOR;
            break;
        case '$':
            regex_node->regex.type = END;
            break;
        case '[':
            while (*t != '\0' && *(++t) != ']') {
                group_size++;
            }

            if (group_size > 0) {
                regex_node->regex.group_to_match = (char *)malloc(group_size);

                group_size = 0;
                for (t = atom + 1; *t != ']'; t++) {
                    regex_node->regex.group_to_match[group_size++] = *t;
                }

            }

            regex_node->regex.type = REG;
            regex_node->regex.size = GROUP;
            regex_node->regex.group_size = group_size;
            break;
        case '.':
            regex_node->regex.type = WILD;
            regex_node->regex.size = SINGLE;
            break;
        default:
            regex_node->regex.type = REG;
            regex_node->regex.to_match = *atom;
            regex_node->regex.size = SINGLE;
            break;
    }

    return regex_node;
}

enum regex_type_e is_repeated(char *atom)
{
    enum regex_type_e type = ERROR;
    int repeated = 0;
    char *t = atom;

    if (*t++ == '[') {
        while (*t != '\0' && *t++ != ']');
    }

    if (*t == '+') {
        type = PLUS;
    } else if (*t == '*') {
        type = STAR;
    }

    return type;
}

struct compiled_regex_t * clone(struct compiled_regex_t *node)
{
    struct compiled_regex_t *cloned = malloc_regex();
    int i;

    cloned->regex.type = node->regex.type;
    cloned->regex.size = node->regex.size;
    cloned->regex.to_match = node->regex.to_match;
    cloned->regex.group_size = node->regex.group_size;

    if (node->regex.group_to_match != NULL) {
        cloned->regex.group_to_match = (char *)malloc(node->regex.group_size);
        
        for (i = 0; i < node->regex.group_size; i++) {
            cloned->regex.group_to_match[i] = node->regex.group_to_match[i];
        }
    }

    return cloned;
}

struct compiled_regex_t * last_node(struct compiled_regex_t *head)
{
    struct compiled_regex_t *last = head;

    while (last != NULL && last->next != NULL) {
        last = last->next;
    }

    return last;
}

char * next_atom(char *regex)
{
    char *t = regex;

    if (*t != '\0' && *t != '[') {
        for (t = regex + 1; *t != '\0' && !regex_is_token(*t); t++); // skip star and plus
    } else if (*t != '\0'){
        for (t = regex + 1; *t != '\0' && t[-1] != ']'; t++); // find the closing bracket
        while (*t != '\0' && (*t == '*' || *t == '+')) { t++; } // move past stars and pluses
    }

    return t;
}

int regex_is_token(char t)
{
    int is_token = 0;

    if (isalnum(t)) {
        is_token = 1;
    } else {
        switch (t) {
            case '^':
            case '$':
            case '.':
            case '[':
            case ' ':
                is_token = 1;
            default:
                break;
        }
    }

    return is_token;
}

void print_compiled_regex(struct compiled_regex_t *regex)
{
    int regex_num = 1;
    int i;

    while (regex != NULL) {
        printf("%d: Type is ", regex_num);
        
        switch (regex->regex.type) {
            case REG:
                printf("REG");
                break;
            case STAR:
                printf("STAR");
                break;
            case PLUS:
                printf("PLUS");
                break;
            case WILD:
                printf("WILD");
                break;
            case ANCHOR:
                printf("ANCHOR");
                break;
            case END:
                printf("END");
                break;
            case ERROR:
            default:
                printf("Not initialized :( ");
                break;
        }

        printf(", Size is ");

        switch (regex->regex.size) {
            case SINGLE:
                printf("SINGLE");
                break;
            case GROUP:
                printf("GROUP");
                break;
            default:
                printf("Not initialized :( ");
                break;
        }

        if (regex->regex.group_to_match == NULL && 
           (regex->regex.type == REG || regex->regex.type == STAR)) {
            printf(", Character to match is %c", regex->regex.to_match);
        } else if (regex->regex.type == WILD) {
            printf(", Is wildcard");
        } else if (regex->regex.group_to_match != NULL) {
            printf(", Group size is %d", regex->regex.group_size);
            printf(", Group to match is [");

            for (i = 0; i < regex->regex.group_size; i++) {
                printf("%c", regex->regex.group_to_match[i]);
            }
            
            printf("]");
        }

        printf("\n");

        regex_num++;
        regex = regex->next;
    }
}

void free_compiled_regex(struct compiled_regex_t *regex)
{
    struct compiled_regex_t *temp = regex;

    while (temp != NULL) {
        regex = regex->next;

        if (temp->regex.group_to_match != NULL) {
            free(temp->regex.group_to_match);
        }

        free(temp);

        temp = regex;
    }
}

static int regex_match_compiled_loop(struct compiled_regex_t *regex, char *string)
{
    int next, i;
    char *t = string;

    while (regex != NULL) {
        switch (regex->regex.type) {
            case STAR:
                return regex_match_compiled_star(regex, t);
                break;
            case END:
                if (*t != '\0') {
                    return 0;
                } else {
                    regex = regex->next;
                }
                break;
            case WILD:
            case REG:
                if (regex_match_compiled_group(regex, t)) {
                    regex = regex->next;
                    t++;
                } else {
                    return 0;
                }
                break;
        }
    }

    return 1;
}

static int regex_match_compiled_star(struct compiled_regex_t *regex, char *string)
{
    char *t = string;

    while (*t != '\0' && regex_match_compiled_group(regex, t)) {
        t++;
    }

    do {
        if (regex_match_compiled_loop(regex->next, t)) {
            return 1;
        }
    } while (t-- > string);

    return 0;
}

static int regex_match_compiled_group(struct compiled_regex_t *regex, char *string) 
{
    int i;

    if (regex->regex.type == WILD) {
        return *string != '\0';
    } else if (regex->regex.size == SINGLE) {
        return regex->regex.to_match == *string;
    }

    for (i = 0; i < regex->regex.group_size; i++) {
        if (*string == regex->regex.group_to_match[i]) {
            return 1;
        }
    }

    return 0;
}
