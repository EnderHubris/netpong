#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int stringCount;
    char** strs;
} Strings;

/**
 * Split a string based on a delimiter and return
 * an array of sub-strings
 */
Strings SplitStr(char* s, char t);

#endif