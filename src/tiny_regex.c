/*--------------------------------------------------------------------
@file            : tiny_regex.c
@brief           :
----------------------------------------------------------------------
@author          :
 Release Version : 0.0.1
 Release Date    : 2023/12/14
----------------------------------------------------------------------
@attention       :
Copyright [2023] [copyright holder]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

--------------------------------------------------------------------*/
#include "tiny_regex.h"
#include "stdio.h"

// DEFINE -------------------

#define TINYREGEX_MATCH_CAPITAL     'A'
#define TINYREGEX_MATCH_LOWERCASE   'a'

#define TINYREGEX_MATCH_NULL        '\0'

#define TINYREGEX_FLAG_STAT            0x01
#define TINYREGEX_FLAG_ONEOFTHEM       0x02
#define TINYREGEX_FLAG_ONEOFTHEM_NOT   0x04
#define TINYREGEX_FLAG_MODE_CHECK      0x38
#define TINYREGEX_FLAG_SPACE           0x08  // 0b00001000
#define TINYREGEX_FLAG_SPACE_NOT       0x10  // 0b00010000
#define TINYREGEX_FLAG_WORD            0x18  // 0b00011000
#define TINYREGEX_FLAG_WORD_NOT        0x20  // 0b00100000
#define TINYREGEX_FLAG_DIGITAL         0x28  // 0b00101000
#define TINYREGEX_FLAG_DIGITAL_NOT     0x30  // 0b00110000
#define TINYREGEX_FLAG_ALL             0x38  // 0b00111000
#define TINYREGEX_FLAG_REPEAT          0x40 

typedef struct {
    char* Data;
    uint32_t Len;
}tr_match_str_t;

typedef struct {
    int16_t Min;
    int16_t Max;
    int Num;
}tr_repeat_t;
typedef struct {
    tr_match_node_t NodePool[TINY_REGEX_CONFIG_CACHEPOOL_SIZE];
    uint32_t NodePoolIndex;

}tr_parameters_t;
tr_parameters_t tr_Params = { 0 };
static tr_parameters_t* const base = &tr_Params;


// static inline tr_re_t tr_getSubPatter(const char* _val, const uint32_t _len);
// static inline tr_re_t tr_getSubPatterForward(const char* _val, const uint32_t _len);
static inline tr_match_node_t* __tr_newnode(char _val);
static inline char* __match_str_normal(tr_match_str_t _src, tr_match_str_t _des);


/* fn   : tregex_complie
 * des  :
 * args :
 * res  :
 */
tr_re_t tregex_complie(const char* _val, const uint32_t _len)
{
    tr_re_t res = { .Count = 0,.NodePool = NULL };
    tr_match_node_t* curnode = 0, * newnode = 0, * bufnode = 0;
    uint32_t offset = 0;
    uint16_t count_parenthesis_l = 0, count_parenthesis_r = 0;
    uint8_t flag = 0; // 1:create new node 0x02:add character
    uint8_t subpattern = 0, item_level;
    char pre[2], esc = 0;
    if (_val == NULL || _len == 0) {
        goto end;
    }

    base->NodePoolIndex = 0;
    flag = 0x01;

    while (offset < _len) {
        if (esc == 1) {
            esc = 0;
            switch (_val[offset]) {
            case 'w':case 'W':case 'd':case 'D':case 's':case 'S': {
                curnode->OP = _val[offset];
            }break;
            case '(':case ')': case '[':case ']':case '{':case '}':
            case '^':case '$':case '.':case '?':case '*':case '+':
            case '|':case '\\': {
                curnode->SubStr.Cache = (char*)(_val + offset);
                curnode->SubStr.Size = 1;
            }break;
            default: {
                curnode->SubStr.Cache = (char*)(_val + offset - 1);
                curnode->SubStr.Size += 2;
            }break;
            }
        }
        else {
            switch (_val[offset]) {
            case '(': {
                newnode = __tr_newnode(TINYREGEX_MATCH_NULL);
                curnode = newnode;
                curnode->Sta.Level = ++item_level;
                flag &= (~0x1);
            }break;
            case ')': {
                item_level--;
                flag |= 0x1;
            }break;
            case '?':case '*':case '+': case '{': {
                switch (pre[1]) {
                case ')': {
                    newnode = __tr_newnode(_val[offset]);
                    curnode = newnode;
                    curnode->Sta.Level = item_level + 1;
                }break;
                case 'w':case 'W':case 'd':case 'D':case 's':case 'S': {
                    if (pre[0] == '\\') {
                        curnode->SubStr.C = pre[1];
                        curnode->OP = _val[offset];
                    }
                }break;
                default: {
                    curnode->SubStr.Size--;
                    newnode = __tr_newnode(_val[offset]);
                    newnode->Sta.Level = item_level;
                    newnode->SubStr.Cache = (char*)&_val[offset - 1];
                    newnode->SubStr.Size = 1;
                    curnode = newnode;
                }break;
                }
                if (_val[offset] == '{') {
                    curnode->Repeat.Min = 0;
                    curnode->Repeat.Max = 0;
                    while (_val[offset] != '}') {
                        pre[0] = pre[1];
                        pre[1] = _val[offset];
                        offset++;
                        if (_val[offset] == ',') {
                            subpattern = 1;
                            curnode->Repeat.Max = -1;
                            continue;
                        }
                        else {
                            if (subpattern == 0) {
                                curnode->Repeat.Min *= 10;
                                curnode->Repeat.Min = _val[offset] - '0';
                            }
                            else {
                                if (curnode->Repeat.Max == -1) {
                                    curnode->Repeat.Max = 0;
                                }
                                curnode->Repeat.Max *= 10;
                                curnode->Repeat.Max = _val[offset] - '0';
                            }
                        }
                    }
                }
                else {
                    if (_val[offset] == '?' || _val[offset] == '*') {
                        curnode->Repeat.Min = 0;
                    }
                    if (_val[offset] == '*' || _val[offset] == '+') {
                        curnode->Repeat.Max = -1;
                    }
                }
            }break;
            case '[': {
                newnode = __tr_newnode(_val[offset]);
                curnode = newnode;
                curnode->Sta.Level = item_level;
                curnode->SubStr.Cache = (char*)&_val[offset + 1];
                do {
                    curnode->SubStr.Size++;
                    pre[0] = pre[1];
                    pre[1] = _val[offset];
                    offset++;
                } while (_val[offset] != ']');
                curnode->SubStr.Size -= 1;
            }break;
            case '|': {
                newnode = __tr_newnode(_val[offset]);
                curnode = newnode;
                curnode->Sta.Level = item_level;
                flag |= 0x1;
            }break;
            case '^': {
                newnode = __tr_newnode(_val[offset]);
                curnode = newnode;
                curnode->Sta.Level = item_level;
            }break;
            case '$': {
                newnode = __tr_newnode(_val[offset]);
                curnode = newnode;
                curnode->Sta.Level = item_level;
            }break;
            case '\\': {
                if (esc == 0) {
                    newnode = __tr_newnode(_val[offset]);
                    curnode = newnode;
                    curnode->Sta.Level = item_level;
                    esc = 1;
                }
            }break;
            default: {
                flag |= 0x02;
            }break;
            }
        }
        if ((flag & 0x02) != 0) {
            flag &= ~0x02;
            if ((flag & 0x01) != 0) {
                flag &= ~0x01;
                newnode = __tr_newnode(TINYREGEX_MATCH_NULL);
                curnode = newnode;
                curnode->Sta.Level = item_level;
            }
            if (curnode->SubStr.Cache == NULL) {
                curnode->SubStr.Cache = (char*)(_val + offset);
                curnode->SubStr.Size = 0;
            }
            curnode->SubStr.Size++;
        }
        if (base->NodePoolIndex >= TINY_REGEX_CONFIG_CACHEPOOL_SIZE) {
            goto end;
        }


        pre[0] = pre[1];
        pre[1] = _val[offset];
        offset++;
    }

    char buf[10] = { '\0' };
    for (int i = 0;i < base->NodePoolIndex;i++) {
        bufnode = &base->NodePool[i];
        memset(buf, 0, 10);
        if (bufnode->SubStr.Cache != 0 && bufnode->SubStr.Size != 0) {
            memcpy(buf, bufnode->SubStr.Cache, bufnode->SubStr.Size);
            printf("0x%x - Level : %d - op : %c text : %s\r\n", bufnode, bufnode->Sta.Level, bufnode->OP, buf);
        }
        else {
            printf("0x%x - Level : %d - op : %c C : %c\r\n", bufnode, bufnode->Sta.Level, bufnode->OP, bufnode->SubStr.C);
        }
    }

    char test0[] = "icknabababcaabbccabcabfa";
    char test_t[] = "ababc";
    tr_match_str_t str[2];
    str[0].Data = test0;
    str[0].Len = strlen(test0);
    str[1].Data = test_t;
    str[1].Len = strlen(test_t);

    char* test_str = __match_str_normal(str[0], str[1]);
    if (test_str != NULL) {
        printf("match : %s\r\n", test_str);
    }

    res.NodePool = base->NodePool;
    res.Count = base->NodePoolIndex;
end:
    return res;
}

static inline tr_match_node_t* __tr_newnode(char _val)
{
    tr_match_node_t* res = 0;
    res = &base->NodePool[base->NodePoolIndex];
    base->NodePoolIndex++;
    res->OP = _val;
    res->SubStr.Cache = 0;
    res->SubStr.Size = 0;
    res->Repeat.Min = 1;
    res->Repeat.Max = 1;

    return res;
}

char* tregex_match_pat(const char* _srcstr, const uint32_t _slen, tr_re_t _pat)
{
    char* res = NULL;
    uint32_t index = 0, offset = 0;
    if (_srcstr == NULL || _slen == 0 || _pat.Count == 0 || _pat.NodePool == NULL) {
        goto end;
    }
    while (index < _pat.Count) {
        switch (_pat.NodePool[index].OP) {
        case TINYREGEX_MATCH_NULL: {

        }break;
        default:break;
        }
    }


end:
    return res;
}

/*
 *   _src :        sources character data
 *  _checknum :    will check number
 *  _c:            want to check the charcter
 * _direction:     0 - go to start
 *                 1 - go to end
 * _continue:      0 - characters do not have to be consecutive.
 *                 1 - characters must be contiguous in the string
 *
 *  return:        the _c number in _src
 */
uint8_t __checkCharNum(const char* _src, uint8_t _checknum, char _c, uint8_t _direction, uint8_t _continue)
{
    uint8_t res = 0;
    uint8_t index = 0;
    switch (_direction) {
    case 0: {  // backward
        index = 0;
        while (index < _checknum) {
            if (((char*)(_src + index))[0] == _c) {
                res++;
            }
            else if (_continue != 1) {
                break;
            }
            index++;
        }
        }break;
        case 1: {  // forward
        index = 0;
        while (index < _checknum) {
            if (((char*)(_src - index))[0] == _c) {
                res++;
            }
            else if (_continue != 1) {
                break;
            }
            index++;
        }
        }break;
        default:break;
    }

    return res;
}

static inline uint32_t __getMatchType(char _c)
{
    uint32_t res = 0;
    switch (_c)
    {
        case 's':res = TINYREGEX_FLAG_SPACE;break;
        case 'S':res = TINYREGEX_FLAG_SPACE_NOT;break;
        case 'w':res = TINYREGEX_FLAG_WORD;break;
        case 'W':res = TINYREGEX_FLAG_WORD_NOT;break;
        case 'd':res = TINYREGEX_FLAG_DIGITAL;break;
        case 'D':res = TINYREGEX_FLAG_DIGITAL_NOT;break;
        default:break;
    }
    return res;
}

static inline uint8_t __checkCharacterWithGlobbing(const char _c,uint32_t _flag)
{
    uint8_t res = 0xFF;
    switch (_flag & TINYREGEX_FLAG_MODE_CHECK) {
        case TINYREGEX_FLAG_SPACE: {
            if (_c == ' ' || _c == '\r' || _c == '\n' || _c == '\t' || _c == '\f' || _c == 'v') {res = 0;}
        }break;
        case TINYREGEX_FLAG_SPACE_NOT: {
            if (_c != ' ' && _c != '\r' && _c != '\n' && _c != '\t' && _c != '\f' && _c != 'v') {res = 0;}
        }break;
        case TINYREGEX_FLAG_WORD: {
            if ('a' <= _c && _c <= 'z') { res = 0; }
            else if ('A' <= _c && _c <= 'Z') {res = 0;}
            else if (_c == '_') { res = 0;break; }
        }
        case TINYREGEX_FLAG_DIGITAL: {
            if ('0' <= _c && _c <= '9') { res = 0; }
        }break;
        case TINYREGEX_FLAG_WORD_NOT: {
            if (('a' > _c || _c > 'z') &&('A' > _c || _c > 'Z') &&('0' > _c || _c > '9') &&_c != '_') {res = 0;}
        }break;
        case TINYREGEX_FLAG_DIGITAL_NOT:{
            if ('0' > _c || _c > '9') {res = 0;}
        }break;
        case TINYREGEX_FLAG_ALL: {
            if (' ' <= _c && _c <= '`') { res = 0; }
#if TINY_REGEX_CONFIG_DOT_IGNORE_NEWLINE == 1
            else if (_c == '\r' || _c == '\n') {res = 0;}
#endif
        }break;
        default:break;
    }

    return res;
}

static inline uint8_t __getRepeatByBraces(char *_src,uint8_t _len,tr_repeat_t *_repeat)
{
    uint8_t res = 0;
    uint8_t i = 0, side = 0;
    int16_t val = 0;
    _repeat->Min = 0;
    _repeat->Max = -1;
    _repeat->Num = 0;
    while (i < _len && _src[i] != '}') {
        if (_src[i] == ',') {
            side = 1;
            val = 0;
        }
        else if('0' <= _src[i] && _src[i] <= '9'){
            val *= 10;
            val = _src[i] - '0';
            if (side == 0) {
                _repeat->Min = val;
            }
            else if (side == 1) {
                _repeat->Max = val;
            }
        }
        i++;
    }
    res = i;
    return res;
}

static inline uint8_t __checkOneOfThem(const _c,char* _val,uint32_t _len,uint8_t _not)
{
    uint8_t res = 0xFF;
    for (uint32_t i = 0;i < _len;i++) {
        switch (_val[i]) {
        case '-': {
            if (i+1 < _len && _c <= _val[i + 1]) {
                res = 0;
                goto end;
            }
        }break;
        case '\\': {

        }break;
        default:break;
        }
    }
end:
    if (_not == 1) {
        res = res == 0 ? 1 : 0;
    }
    return res;
}

tr_res_t tregex_match_str(const char* _srcstr, uint32_t _slen, const char* _pattern, uint32_t _plen)
{
    tr_res_t res = { .Data = NULL,.Size = 0 };
    uint32_t s_index = 0, p_index = 0, i_repeat = 0,len_subitem = 0;
    uint8_t curlevel = 0, misscout = 0,  // 
        check = 0; // 1 : normal check 2 : globbing check
    tr_repeat_t repeat[TINY_REGEX_CONFIG_DEPTH_LEVEL];
    char prechar = '\0';
    char* startpos = NULL,subitem = NULL;
    uint32_t flag = 0;
    if (_srcstr == NULL || _pattern == NULL) {
        goto end;
    }
    if (_slen == 0) { _slen = strlen(_srcstr); }
    if (_plen == 0) { _plen = strlen(_pattern); }

    while (p_index <= _plen) {
        switch (_pattern[p_index]) {
        case '^': {
            if (p_index != 0 || (curlevel != 0 && __checkCharNum(&_pattern[p_index],p_index,'(',1,1) != curlevel)) {
                goto end;
            }
            s_index = 0;
            flag |= TINYREGEX_FLAG_STAT;
        }break;
        case '\\': {
            if (p_index < _plen) {
                p_index++;
            }
            switch (_pattern[p_index]) {
                case 's':case 'S':case 'w':case 'W':case 'd':case 'D': {
                    flag |= __getMatchType(_pattern[p_index]);
                    check = 2;
                }break;
                default:check = 1;break;
            }
        }break;
        case '.': {
            flag |= __getMatchType(_pattern[p_index]);
            check = 2;
        }break;
        case '[': {
            p_index += 1;
            if (p_index < _plen) {
                subitem = &_pattern[p_index];
                len_subitem = 0;
            }
            else {
                goto end;
            }
            for (;p_index < _plen;p_index++) {
                if (_pattern[p_index] == ']') {
                    break;
                }
                len_subitem++;
            }
            flag |= TINYREGEX_FLAG_ONEOFTHEM;
            check = 4;
        }break;
        case '?': {
            repeat[curlevel].Min = 0;
            repeat[curlevel].Max = 1;
            repeat[curlevel].Num = 0;
            check = 3;
        }break;
        case '*': {
            repeat[curlevel].Min = 0;
            repeat[curlevel].Max = -1;
            repeat[curlevel].Num = 0;
            check = 3;
        }break;
        case '+': {
            repeat[curlevel].Min = 1;
            repeat[curlevel].Max = -1;
            repeat[curlevel].Num = 0;
            check = 3;
        }break;
        case '{': {
            p_index += __getRepeatByBraces((const char*)&_pattern[p_index], _plen - p_index, &repeat[curlevel]);
            check = 3;
        }break;
        default: check = 1;break;
        }
        // match  ---------
        if (misscout == 1 && check != 3) {
            break;
        }
        switch (check) {
        case 0x01: {
            check = 0;
            misscout = 0;
            prechar = _pattern[p_index];
            while (s_index <= _slen ) {
                if (prechar == _srcstr[s_index]) {
                    if (startpos == NULL) {startpos = (char*)&_srcstr[s_index];}
                    s_index++;
                    break;
                }
                else if (startpos != NULL || (flag & TINYREGEX_FLAG_STAT !=0)) {check = 0x80;break;}
                s_index++;
            }
            flag &= ~TINYREGEX_FLAG_MODE_CHECK;
        }break;
        case 0x02: {
            check = 0;
            misscout = 0;
            while (s_index <= _slen) {
                if (__checkCharacterWithGlobbing(_srcstr[s_index], flag & TINYREGEX_FLAG_MODE_CHECK) == 0) {
                    if (startpos == NULL) {startpos = (char*)&_srcstr[s_index];}
                    s_index++;
                    break;
                }
                else if (startpos != NULL) {check = 0x80;break;}
                s_index++;
            }
        }break;
        case 0x03: {
            check = 0;
            if (misscout == 0) {
                repeat[curlevel].Num = 1;
                if ((flag & TINYREGEX_FLAG_MODE_CHECK) != 0) {
                    while (s_index < _slen && (repeat[curlevel].Max < 0 || repeat[curlevel].Num < repeat[curlevel].Max)) {
                        if (__checkCharacterWithGlobbing(_srcstr[s_index], flag & TINYREGEX_FLAG_MODE_CHECK) != 0) {
                            break;
                        }
                        s_index++;
                        repeat[curlevel].Num++;
                    }
                }
                else {
                    while (s_index < _slen && (repeat[curlevel].Max < 0 || repeat[curlevel].Num < repeat[curlevel].Max)) {
                        if (prechar != _srcstr[s_index]) {
                            break;
                        }
                        s_index++;
                        repeat[curlevel].Num++;
                    }
                }
            }
            else {
                repeat[curlevel].Num = 0;
            }
            if (repeat[curlevel].Min > repeat[curlevel].Num) {
                check |= 0x80;    // match fail
            }
            flag &= ~(TINYREGEX_FLAG_MODE_CHECK | TINYREGEX_FLAG_ONEOFTHEM);
        }break;
        case 0x04: {

        }break;
        default:break;
        }
        if ((check & 0x80) != 0) {
            // 
            
            misscout++;
            if ((flag & TINYREGEX_FLAG_STAT) != 0) {
                break;
            }
            if (misscout == 2) {
                res.Data = NULL;
                res.Size = 0;
                goto end;
            }
        }
        p_index++;
    }

    if (startpos == NULL) {
        goto end;
    }
    res.Data = startpos;
    res.Size = s_index - (uint32_t)(((unsigned int)startpos) - ((unsigned int)_srcstr));
end:
    printf("  finished :%d\r\n",s_index);
    return res;
}



static inline char* __match_str_normal(tr_match_str_t _src, tr_match_str_t _des)
{
    char* res = NULL;
    //res = kmp(_src.Data, _des.Data);
end:
    return res;
}
#if 0

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
        case '}': { if (pre[1] != '\\') { count_brace--; range_flag = 2; } }break;
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
                pre[1] == '+' || pre[1] == '?' || pre[1] == '*' ||
                pre[1] == '^' || pre[1] == '|') {
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
            if (count_brace != 0 && range_flag < 2) {
                if (',' == _val[i] && range_flag == 0) {
                    range_flag = 1;
                }
                else {
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

tr_re_t tregex_match(const char* _srcstr, const uint32_t _slen, const char* _pattern, const uint32_t _plen)
{
    tr_re_t res = { .Buf = NULL,.Len = 0 };
    tr_re_t subpattern[2];
    char pre[2] = { 0 }, spec = 0;
    tr_match_type_t matchtype = match_type_none;
    uint32_t index = 0, offset = 0, flag = 0, checkonce = 0;
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
                    spec = 'w';
                    if (('0' <= _srcstr[index] && _srcstr[index] <= '9') ||
                        ('a' <= _srcstr[index] && _srcstr[index] <= 'z') ||
                        ('A' <= _srcstr[index] && _srcstr[index] <= 'Z') ||
                        _srcstr[index] == '_') {
                        index++;
                    }
                }
            }break;
            case 'W': {
                if (pre[1] == '\\' && pre[0] != '\\') {
                    spec = 'W';
                    if (('0' >= _srcstr[index] || _srcstr[index] >= '9') &&
                        ('a' >= _srcstr[index] || _srcstr[index] >= 'z') &&
                        ('A' >= _srcstr[index] || _srcstr[index] >= 'Z') &&
                        _srcstr[index] != '_') {
                        index++;
                    }
                }
            }
            case 'd': {
                if (pre[1] == '\\' && pre[0] != '\\') {
                    spec = 'd';
                    if ('0' <= _srcstr[index] && _srcstr[index] <= '9') {
                        index++;
                    }
                }
            }break;
            case 'D': {
                if (pre[1] == '\\' && pre[0] != '\\') {
                    spec = 'D';
                    if ('0' >= _srcstr[index] && _srcstr[index] >= '9') {
                        index++;
                    }
                }
            }break;
            case '+': {

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

static inline tr_re_t tr_getSubPatter(const char* _val, const uint32_t _len)
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
            if (*(_val - i) == base->Prioriy[j]) {
                goto end;
            }
        }
        res.Len++;
    }

end:
    return res;
}

#endif
