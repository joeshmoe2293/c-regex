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
    print_match(regex, string);

    char regex2[] = "[bad]*";
    char string2[] = "baaaad";
    print_match(regex2, string2);

    char regex3[] = "^a[bad]*$";
    char string3[] = "baaaad";
    print_match(regex3, string3);

    return 0;
}
