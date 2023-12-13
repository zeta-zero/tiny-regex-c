
#include "tiny_regex.h"
#include "string.h"
#include "stdint.h"

typedef enum {
    match_type_none = 0x00,
    match_type_start = 0x01,
    match_type_end = 0x02,
    match_type_notinclude,
}tr_match_type_t;

#define MATCH_LIMIT_START  0x01
#define MATCH_LIMIT_END    0x02
#define MATCH_NORMAL       0x04
#define MATCH_SPEC         0x08

typedef enum {
    match_mod_normal = 0x00,
    match_mod_more_string,
    match_mod_more_single,
    match_mod_more_spec,
    match_mod_more_spec_list,
    match_mod_or_string,
    match_mod_or_single,
}tr_match_mod_t;

typedef struct {
    char Prioriy[];

}tr_parameters_t;
tr_parameters_t tr_Params = {
    // # it means any Escape character : \s \S \w \W \d \date
    .Prioriy = {'\\' , '(' , ')' , '[' , ']' , '*' , '+' , '?','{','}','^','$' ,'.'  , '|'},
};
static tr_parameters_t* const base = &tr_Params;

static inline tr_re_t tr_getSubPatter(const char* _val, const uint32_t _len);
static inline tr_re_t tr_getSubPatterForward(const char* _val, const uint32_t _len);

/* check pattern */
static inline uint8_t tregex_checkPattern(const char* _val, const uint32_t _len)
{
    uint8_t res = 0;
    char pre[2] = { 0 };
    int count_parenthesis = 0, count_squarebrackets = 0, count_brace = 0,
        count_zero = 0;
    uint8_t range_flag = 0;
    if (_val == NULL || _len == 0) {
        res = 1;  // empty parameter
        goto end;
    }
    for (uint32_t i = 0;i < _len;i++) {
        switch (_val[i]) {
        case '\0': { if (i == count_zero)count_zero++; }break;
        case '[': { if (pre[1] != '\\')count_squarebrackets++; }break;
        case ']': { if (pre[1] != '\\')count_squarebrackets--; }break;
        case '{': { if (pre[1] != '\\') { count_brace++; range_flag = 0; } }break;
        case '}': { if (pre[1] != '\\') {count_brace--; range_flag = 2;} }break;
        case '(': { if (pre[1] != '\\')count_parenthesis++; }break;
        case ')': { if (pre[1] != '\\')count_parenthesis--; }break;
        case '^': {
            if (i == _len - 1) {
                res = 4; // bad pattern, after ^ have not any character
                goto end;
            }
            else if (count_zero == i - 1) {
                if ((pre[1] == '(' && _val[i + 1] != ')') ||
                    (pre[1] == '[' && _val[i + 1] != ']')) {
                    break;
                }
            }
            else if (count_zero != i) {
                if ((pre[1] == '[' && _val[i + 1] != ']') && pre[0] != '\\') {
                    break;
                }
            }
            else if (pre[1] == _val[i]) {
                res = 8; // same character as before
                goto end;
            }
            res = 3; // bad pattern, before ^ have character
            goto end;
        }break;
        case '$': {
            if (i < _len - 1 && _val[i] != 0) {
                res = 5; // bad pattern, after $ have character
                goto end;
            }
            else if (i == 0) {
                res = 6; // bad pattern, before $ have not any character
                goto end;
            }
            else if (pre[1] == _val[i]) {
                res = 8; // same character as before
                goto end;
            }
        }break;
        case '|': {
            if (pre[1] == 0) {
                res = 7; // bad pattern, before | have not any character
            }
            else if (pre[1] == _val[i]) {
                res = 8; // same character as before
            }
        }break;
        case '+':
        case '?':
        case '*': {
            if (pre[1] == '{' || pre[1] == '}' || pre[1] == '[' || pre[1] == '(' ||
                pre[1] == '+' || pre[1] == '?'|| pre[1] == '*' ||
                pre[1] == '^' || pre[1] == '|' ) {
                res = 10; // error character before
                goto end;
            }
        }break;
        case '.': {
            if (pre[1] == '^') {
                res = 10; // error character before
                goto end;
            }
        }break;
        default: {
            if (count_brace != 0 && range_flag<2) {
                if (',' == _val[i] && range_flag == 0) {
                    range_flag = 1;
                }
                else{
                    if ('0' > _val[i] || _val[i] > '9') {
                        res = 11; //  error param of {} 
                    }
                }
            }
        }break;
        }
        if (count_squarebrackets < 0 || count_brace < 0 || count_parenthesis < 0) {
            res = 2; // bracket mismatch
            goto end;
        }
        else if (count_squarebrackets > 1 || count_brace > 1) {
            res = 9; // error character nesting
            goto end;
        }
        pre[0] = pre[1];
        pre[1] = _val[i];
    }
    if (count_squarebrackets + count_brace + count_parenthesis != 0) {
        res = 2; // bracket mismatch
        goto end;
    }

end:
    return res;
}

tr_re_t tregex_match(const char* _srcstr,const uint32_t _slen,const char *_pattern,const uint32_t _plen)
{
    tr_re_t res = { .Buf = NULL,.Len = 0 };
    tr_re_t subpattern[2];
    char pre[2] = { 0 };
    tr_match_type_t matchtype = match_type_none;
    uint32_t index = 0,offset = 0,flag = 0,checkonce = 0;
    if (_srcstr == NULL || _slen == 0 || _pattern == NULL || _plen == 0) {
        goto end;
    }
    while (index < _slen) {
        checkonce = 0;
        while (offset < _plen) {
            switch (_pattern[offset]) {
            case '^': {
                if (pre[1] == '[') {
                }
                else {
                    flag |= MATCH_LIMIT_START;
                }
                flag |= MATCH_NORMAL;
            }break;
            case 'w': {
                if (pre[1] == '\\' && pre[0] != '\\') {
                    if (('0' <= _srcstr[index] && _srcstr[index] <= '9') ||
                        ('a' <= _srcstr[index] && _srcstr[index] <= 'z') ||
                        ('A' <= _srcstr[index] && _srcstr[index] <= 'Z') ||
                        _srcstr[index] == '_') {
                        index++;
                    }
                }

            }break;
            default: {
                if (flag == 0) {
                    subpattern[0].Buf = &_pattern[offset];
                    subpattern[0].Len = 1;
                    flag |= MATCH_NORMAL;
                }
                else if ((flag & MATCH_NORMAL) != 0) {
                    subpattern[0].Len++;
                }
            }break;
            }
            pre[0] = pre[1];
            pre[1] = _pattern[offset];
            offset++;
            if (checkonce == 1) {
                break;
            }
        }
        while (index < _slen) {
            if (memcmp(&_srcstr[index], subpattern[0].Buf, subpattern[0].Len) == 0) {
                break;
            }
            if ((flag & MATCH_LIMIT_START) != 0) {
                res.Buf == NULL;
                res.Len = 0;
                goto end;
            }
            index += subpattern[0].Len;
        }
        


        
    }


end:
    return res;
}

static inline tr_re_t tr_getSubPatter(const char *_val,const uint32_t _len)
{
    tr_re_t res = { .Buf = _val,.Len = 0 };
    for (uint32_t i = 0;i < _len;i++) {
        for (uint8_t j = 0;j < 14;j++) {
            if (_val[i] == base->Prioriy[j]) {
                goto end;
            }
        }
        res.Len++;
    }

end:
    return res;
}


static inline tr_re_t tr_getSubPatterForward(const char* _val, const uint32_t _len)
{
    tr_re_t res = { .Buf = _val,.Len = 0 };
    for (uint32_t i = 0;i < _len;i++) {
        for (uint8_t j = 0;j < 14;j++) {
            if (*(_val-i) == base->Prioriy[j]) {
                goto end;
            }
        }
        res.Len++;
    }

end:
    return res;
}