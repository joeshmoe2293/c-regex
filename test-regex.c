#include "regex.h"
#include <stdio.h>
#include <string.h>

void print_match(char *regex, char *string)
{
    int match = regex_match_compiled(regex, string);

    printf("regex: %s, string: %s\n", regex, string);
    printf("match: %d\n", match);
}

int main(int argc, char *argv[])
{
    char regex[] = "^b+a*d+$";
    char string[] = "baaaad";
    char regex2[] = "[bad]*";
    char string2[] = "baaaad";
    char regex3[] = "^a[bad]*$";
    char string3[] = "baaaad";

    print_match(regex, string);
    print_match(regex2, string2);
    print_match(regex3, string3);
    
    // For testing on command line!
    if (argc == 3) {
        print_match(argv[1], argv[2]);
    }

    return 0;
}
