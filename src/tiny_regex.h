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
extern "C" {
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

/* Maximum nesting depth supported */
#ifndef TINY_REGEX_CONFIG_DEPTH_LEVEL
#define TINY_REGEX_CONFIG_DEPTH_LEVEL          8
#endif
    


typedef struct{
    char OP;
    struct {
        uint8_t Level : 8;
    }Sta;
    struct {
        int16_t Min;
        int16_t Max;
    }Repeat;
    struct {
        char* Cache;
        union {
            uint32_t Size;
            char C;
        };
    }SubStr;
}tr_match_node_t;

typedef struct tr_match_list{
    tr_match_node_t *NodePool;
    uint32_t Count;
}tr_re_t;

typedef struct {
    char* Data;
    uint32_t Size;
}tr_res_t;


tr_re_t tregex_complie(const char* _val,const uint32_t _len);
tr_res_t tregex_match_str(const char* _srcstr, uint32_t _slen, const char* _pattern, uint32_t _plen);


#ifdef __cplusplus
}
#endif
#endif
