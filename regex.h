#ifndef __REGEX_H__
#define __REGEX_H__

int regex_match(char *regex, char *string);
int regex_match_compiled(char *regex, char *string);

#endif
