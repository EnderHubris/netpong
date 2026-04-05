#include "utils.h"

static char** AddSubStr(Strings item, char* s) {
    char** collection = malloc((item.stringCount + 1) * sizeof(char*));

    for (int i = 0; i < item.stringCount; ++i) {
        collection[i] = item.strs[i];
    }
    free(item.strs);

    collection[item.stringCount] = s;

    return collection;
}

/**
 * Append character into a given string
 */
static char* AppendChar(char* s, char c) {
    // make room for new char and null terminating
    int len = s ? strlen(s) : 0;
    
    char* nStr = malloc(len+2); // make more room
    if (!nStr) return NULL;
    
    // copy old data into new buffer
    for (int i = 0; i < len; ++i) {
        nStr[i] = s[i];
    }

    // append new character
    nStr[len] = c;
    
    // append null-terminator
    nStr[len+1] = '\0';

    return nStr;
}

/**
 * Split a string based on a delimiter and return
 * an array of sub-strings
 */
Strings SplitStr(char* s, char t) {
    Strings items = { 0, NULL };

    char* nStr = NULL;
    
    for (size_t i = 0; i < strlen(s); ++i) {
        if (s[i] == t) {
            // form new string
            items.strs = AddSubStr(items, nStr);
            ++items.stringCount;
            
            nStr = NULL;
            
            continue;
        }
        nStr = AppendChar(nStr, s[i]);
    }
    
    // get trailing string
    items.strs = AddSubStr(items, nStr);
    ++items.stringCount;
    
    nStr = NULL;
    
    return items;
}