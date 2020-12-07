#include "regex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STR_SIZE 256

int debug = 1;

void print_match(char *regex, char *string)
{
    double time_spent = 0.0;
    clock_t begin = clock(), end;

    int match = regex_match(regex, string);

    end = clock();

    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Calculated in %f seconds ", time_spent);
    if (debug) {
        printf("for regex: %s, string: %s\n", regex, string);
    }
    printf("match: %d\n", match);
}

void print_match_compiled(char *regex, char *string)
{
    double time_spent = 0.0;
    clock_t begin = clock(), end;

    int match = regex_match_compiled(regex, string);

    end = clock();

    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Calculated in %f seconds for ", time_spent);
    if (debug) {
        printf("regex: %s, string: %s\n", regex, string);
    }
    printf("match: %d\n", match);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    char regex[] = "^b+a*d+$";
    char string[] = "baaaad";
    char regex2[] = "[bad]*";
    char string2[] = "baaaad";
    char regex3[] = "^a[bad]*$";
    char string3[] = "baaaad";
    char *fileBuff;
    long fsize;

    if (argc != 2) {
        print_match(regex, string);
        print_match_compiled(regex, string);
        print_match(regex2, string2);
        print_match_compiled(regex2, string2);
        print_match(regex3, string3);
        print_match_compiled(regex3, string3);

        // For testing on command line!
        if (argc == 3) {
            print_match(argv[1], argv[2]);
            print_match_compiled(argv[1], argv[2]);
        }
    } else {
        fp = fopen("test.txt", "r");

        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        fileBuff = (char *)malloc(fsize + 1);
        fread(fileBuff, 1, fsize, fp);
        fclose(fp);
        
        debug = 0;
        print_match(argv[1], fileBuff);
        print_match_compiled(argv[1], fileBuff);

        free(fileBuff);
    }

    return 0;
}
