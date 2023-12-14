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


// #ifndef ZETA_BLIST_NODE_D
// #define ZETA_BLIST_NODE_D
// /* 链表操作基本功能 */
// #define OFFSETOF(_TYPE_,_MEMBER_)                (uint32_t)&(((_TYPE_)0)->_MEMBER_)
// #define CONTRAINER_OF(_PV_,_TYPE_,_MEMBER_)      (_TYPE_)( (uint8_t*)(_PV_) -  OFFSETOF(_TYPE_,_MEMBER_))
// #define LIST_FIRST_ENTRY(_PV_,_TYPE_,_MEMBER_)   CONTRAINER_OF((_PV_)->NextNode,_TYPE_,_MEMBER_)
// #define LIST_INIT(_HEAD_)                        {(_HEAD_)->PreNode = (_HEAD_);(_HEAD_)->NextNode = (_HEAD_);}
// //添加到链表
// #define LIST_ADD(_NEW_,_HEAD_)                   {(_NEW_)->NextNode = (_HEAD_);(_NEW_)->PreNode = (_HEAD_)->PreNode;(_HEAD_)->PreNode->NextNode = (_NEW_);(_HEAD_)->PreNode = (_NEW_);}
// //插入到链表
// #define LIST_INSERT(_NEW_,_HEAD_)                {(_NEW_)->NextNode = (_HEAD_)->NextNode;(_NEW_)->PreNode = (_HEAD_);(_HEAD_)->NextNode->PreNode = (_NEW_);(_HEAD_)->NextNode = (_NEW_);}
// #define LIST_LINK(_PRE_,_NEXT_)                  {(_PRE_)->NextNode = (_NEXT_);(_NEXT_)->PreNode = (_PRE_);}
// #define LIST_DEL(_MEMBER_)                       {LIST_LINK((_MEMBER_)->PreNode,(_MEMBER_)->NextNode);(_MEMBER_)->PreNode = (_MEMBER_);(_MEMBER_)->NextNode = (_MEMBER_);}
// #endif

// #define LIST_FIND_PRE(_PV_)                       CONTRAINER_OF(_PV_->List.PreNode,tr_match_node_t*,List)
// #define LIST_FIND_NEXT(_PV_)                      CONTRAINER_OF(_PV_->List.NextNode,tr_match_node_t*,List)


// // TYPE ---------------------
// #ifndef ZETA_BLIST_NODE_T
// #define ZETA_BLIST_NODE_T
// //bidirectional linked list nodes
// struct zeta_blist_node
// {
// 	struct zeta_blist_node *PreNode;
// 	struct zeta_blist_node *NextNode;
// };
// typedef struct zeta_blist_node zeta_blist_t;
// #endif

typedef struct{
    char* Data;
    uint32_t Len;
}tr_match_str_t;


typedef struct {
    tr_match_node_t NodePool[TINY_REGEX_CONFIG_CACHEPOOL_SIZE];
    uint32_t NodePoolIndex;

}tr_parameters_t;
tr_parameters_t tr_Params = {0};
static tr_parameters_t* const base = &tr_Params;


// static inline tr_re_t tr_getSubPatter(const char* _val, const uint32_t _len);
// static inline tr_re_t tr_getSubPatterForward(const char* _val, const uint32_t _len);
static inline tr_match_node_t* __tr_newnode(char _val);
static inline char* __match_str_normal(tr_match_str_t _src,tr_match_str_t _des);


/* fn   : tregex_complie
 * des  : 
 * args :
 * res  :
 */
tr_re_t tregex_complie(const char* _val,const uint32_t _len)
{
    tr_re_t res = {.Count = 0,.NodePool = NULL};
    tr_match_node_t* curnode = 0, * newnode = 0,*bufnode = 0;
    uint32_t offset = 0;
    uint16_t count_parenthesis_l = 0, count_parenthesis_r = 0;
    uint8_t flag = 0; // 1:create new node 0x02:add character
    uint8_t subpattern = 0,item_level;
    char pre[2], esc = 0;
    if (_val == NULL || _len == 0) {
        goto end;
    }

    base->NodePoolIndex = 0;
    flag = 0x01;

    while(offset < _len)
    {
        if (esc == 1) {
            esc = 0;
            switch (_val[offset]) {
                case 'w':case 'W':case 'd':case 'D':case 's':case 'S': {
                    curnode->OP = _val[offset];
                }break;
                case '(':case ')': case '[':case ']':case '{':case '}':
                case '^':case '$':case '.':case '?':case '*':case '+':
                case '|':case '\\':{
                    curnode->SubStr.Cache = (char*)(_val + offset);
                    curnode->SubStr.Size = 1;
                }break;
                default: {
                    curnode->SubStr.Cache = (char*)(_val + offset - 1);
                    curnode->SubStr.Size+=2;
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
            case '?':case '*':case '+': case '{':{
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
                do{
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
                    esc=1;
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
        if(base->NodePoolIndex >= TINY_REGEX_CONFIG_CACHEPOOL_SIZE){
            goto end;
        }


        pre[0] = pre[1];
        pre[1] = _val[offset];
        offset++;
    }
            
    char buf[10] = {'\0' };
    for (int i = 0;i < base->NodePoolIndex;i++) {
        bufnode = &base->NodePool[i];
        memset(buf, 0, 10);
        if (bufnode->SubStr.Cache != 0&&bufnode->SubStr.Size != 0) {
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

    char *test_str = __match_str_normal(str[0],str[1]);
    if(test_str != NULL){
        printf("match : %s\r\n",test_str);
    }
    
    res.NodePool = base->NodePool;
    res.Count = base->NodePoolIndex;
end:
    return res;
}

static inline tr_match_node_t* __tr_newnode(char _val)
{
    tr_match_node_t* res=0;
    res = &base->NodePool[base->NodePoolIndex];
    base->NodePoolIndex++;
    res->OP = _val;
    res->SubStr.Cache = 0;
    res->SubStr.Size = 0;
    res->Repeat.Min = 1;
    res->Repeat.Max = 1;

    return res;
}

char* tregex_match_pat(const char* _srcstr,const uint32_t _slen,tr_re_t _pat)
{
    char* res = NULL;
    uint32_t index = 0,offset = 0;
    if(_srcstr == NULL || _slen == 0 || _pat.Count == 0|| _pat.NodePool == NULL){
        goto end;
    }
    while(index < _pat.Count){
        switch(_pat.NodePool[index].OP){
            case TINYREGEX_MATCH_NULL:{

            }break;
            default:break;
        }
    }
    

end:
    return res;
}

tr_re_t tregex_match_str(const char* _srcstr,const uint32_t _slen,const char *_pattern,const uint32_t _plen)
{

}

static inline char* __match_str_normal(tr_match_str_t _src,tr_match_str_t _des)
{
    char* res = NULL;
    res = kmp(_src.Data,_des.Data);
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
    char pre[2] = { 0 },spec = 0;
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
                    spec = 'w';
                    if (('0' <= _srcstr[index] && _srcstr[index] <= '9') ||
                        ('a' <= _srcstr[index] && _srcstr[index] <= 'z') ||
                        ('A' <= _srcstr[index] && _srcstr[index] <= 'Z') ||
                        _srcstr[index] == '_') {
                        index++;
                    }
                }
            }break;
            case 'W':{
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
            case 'd':{
                if (pre[1] == '\\' && pre[0] != '\\') {
                    spec = 'd';
                    if ('0' <= _srcstr[index] && _srcstr[index] <= '9') {
                        index++;
                    }
                }
            }break;
            case 'D':{
                if (pre[1] == '\\' && pre[0] != '\\') {
                    spec = 'D';
                    if ('0' >= _srcstr[index] && _srcstr[index] >= '9') {
                        index++;
                    }
                }
            }break;
            case '+':{
                
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

#endif
