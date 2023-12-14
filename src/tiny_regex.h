/*
 * Supports:
 * ---------
 *   '.'        Dot, matches any character
 *   '^'        Start anchor, matches beginning of string
 *   '$'        End anchor, matches end of string
 *   '|'        Or,march left string or right string.
 *   '*'        Asterisk, match zero or more (greedy)
 *   '+'        Plus, match one or more (greedy)
 *   '?'        Question, match zero or one (non-greedy)
 *   '[abc]'    Character class, match if one of {'a', 'b', 'c'}
 *   '[^abc]'   Inverted class, match if NOT one of {'a', 'b', 'c'} -- NOTE: feature is currently broken!
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 */

#ifndef __TINY_REGEX_H__
#define __TINY_REGEX_H__
#ifdef __cplusplus
extern "C"{
#endif
    
#include "string.h"
#include "stdint.h"

/* Define to 1 if there DON'T want '.' to match '\r' + '\n' */
#ifndef TINY_REGEX_CONFIG_DOT_IGNORE_NEWLINE
#define TINY_REGEX_CONFIG_DOT_IGNORE_NEWLINE   1
#endif

/* Limit the size of regular expressions. unit: unit*/
#ifndef TINY_REGEX_CONFIG_CACHEPOOL_SIZE
#define TINY_REGEX_CONFIG_CACHEPOOL_SIZE       400
#endif

typedef struct tr_match_node* tr_re_t;


tr_re_t tregex_complie(const char* _val,const uint32_t _len);



#ifdef __cplusplus
}
#endif
#endif
