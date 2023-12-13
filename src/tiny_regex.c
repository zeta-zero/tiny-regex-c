
#include "tiny_regex.h"
#include "string.h"
#include "stdint.h"

// DEFINE -------------------

#define TINYREGEX_MATCH_CAPITAL     'A'
#define TINYREGEX_MATCH_LOWERCASE   'a'


#ifndef ZETA_BLIST_NODE_D
#define ZETA_BLIST_NODE_D
/* 链表操作基本功能 */
#define OFFSETOF(_TYPE_,_MEMBER_)                (uint32_t)&(((_TYPE_)0)->_MEMBER_)
#define CONTRAINER_OF(_PV_,_TYPE_,_MEMBER_)      (_TYPE_)( (uint8_t*)(_PV_) -  OFFSETOF(_TYPE_,_MEMBER_))
#define LIST_FIRST_ENTRY(_PV_,_TYPE_,_MEMBER_)   CONTRAINER_OF((_PV_)->NextNode,_TYPE_,_MEMBER_)
#define LIST_INIT(_HEAD_)                        {(_HEAD_)->PreNode = (_HEAD_);(_HEAD_)->NextNode = (_HEAD_);}
//添加到链表
#define LIST_ADD(_NEW_,_HEAD_)                   {(_NEW_)->NextNode = (_HEAD_);(_NEW_)->PreNode = (_HEAD_)->PreNode;(_HEAD_)->PreNode->NextNode = (_NEW_);(_HEAD_)->PreNode = (_NEW_);}
//插入到链表
#define LIST_INSERT(_NEW_,_HEAD_)                {(_NEW_)->NextNode = (_HEAD_)->NextNode;(_NEW_)->PreNode = (_HEAD_);(_HEAD_)->NextNode->PreNode = (_NEW_);(_HEAD_)->NextNode = (_NEW_);}
#define LIST_LINK(_PRE_,_NEXT_)                  {(_PRE_)->NextNode = (_NEXT_);(_NEXT_)->PreNode = (_PRE_);}
#define LIST_DEL(_MEMBER_)                       {LIST_LINK((_MEMBER_)->PreNode,(_MEMBER_)->NextNode);(_MEMBER_)->PreNode = (_MEMBER_);(_MEMBER_)->NextNode = (_MEMBER_);}
#endif


// TYPE ---------------------
#ifndef ZETA_BLIST_NODE_T
#define ZETA_BLIST_NODE_T
//bidirectional linked list nodes
struct zeta_blist_node
{
	struct zeta_blist_node *PreNode;
	struct zeta_blist_node *NextNode;
};
typedef struct zeta_blist_node zeta_blist_t;
#endif

typedef struct tr_match_node{
    zeta_blist_t BroList;   // borther list;
    zeta_blist_t FSList;    // father and son list;

    char OP;
    char *Content;
    uint32_t Len;
}tr_match_node_t;

typedef struct {
    zeta_blist_t RootNode;
    tr_match_node_t NodePool[TINY_REGEX_CONFIG_CACHEPOOL_SIZE];
    uint32_t NodePoolCount;

}tr_parameters_t;
tr_parameters_t tr_Params = {0};
static tr_parameters_t* const base = &tr_Params;

static inline tr_re_t tr_getSubPatter(const char* _val, const uint32_t _len);
static inline tr_re_t tr_getSubPatterForward(const char* _val, const uint32_t _len);
static inline tr_match_node_t* __tr_newnode(char _val);


/* fn   : tregex_complie
 * des  : 
 * args :
 * res  :
 */
tr_re_t tregex_complie(char *_val,uint32_t _len){
    tr_re_t res = NULL;
    tr_match_node_t *curnode = 0,*newnode = 0;
    if(_val == NULL || _len == 0){
        goto end;
    }

    LIST_INIT(&base->RootNode);
    base->NodePoolCount = 0;
    curnode = &base->NodePool[0];    LIST_INIT(&curnode->BroList);
    LIST_INIT(&curnode->FSList);
    LIST_ADD(&curnode->FSList,&base->RootNode);
    for(uint32_t i = 0;i<_len;i++){
        switch(_val[i]){
            case '^':{
                newnode = __tr_newnode(_val[i+1]);
                LIST_ADD(&newnode->BroList,&curnode->BroList);
                curnode = newnode;
            }break;
            case '\\':{
                switch(_val[i+1]){
                    case 'w':case 'W':case 's':case 'S':case 'd':case 'D':{
                        newnode = __tr_newnode(_val[i+1]);
                        LIST_ADD(&newnode->BroList,&curnode->BroList);
                        curnode = newnode;
                    }break;
                    case '[':case ']':case '{':case '}':case '(':case ')':
                    case '^':case '$':case '+':case '*':case '?':case '\\':
                    case '|':case '.':{
                        curnode->Len+=2;
                    }break;
                    default:{
                        curnode->Len+=2;
                    }break;
                }
            }break;
            case '(':case '[':case '{':{
                newnode = __tr_newnode(_val[i+1]);
                LIST_ADD(&newnode->FSList,&curnode->FSList);
                curnode = newnode;
            }break;
            case ')':case ']':case '}':{
                curnode = CONTRAINER_OF(&curnode->FSList.PreNode,tr_match_node_t*,FSList);
                newnode = __tr_newnode(0);
                LIST_ADD(&newnode->BroList,&curnode->BroList);
                curnode = newnode;
            }break;
            case '?':case '*':case '+':{
                if(curnode->OP == 0){
                    if(curnode->FSList.NextNode != &curnode->FSList){
                    }
                    else if(curnode->Content != NULL){
                        curnode->Len--;
                        newnode = __tr_newnode(_val[i]);
                        newnode->Content = _val[i-1];
                        newnode->Len++;
                        LIST_ADD(&newnode->BroList,&curnode->BroList);
                        curnode = newnode;
                    }

                }
            }break;
            case '.':{

            }break;
            default:{
                if(curnode->Content == NULL || curnode->Len == 0){
                    curnode->Content = &_val[i];
                    curnode->Len = 0;
                }
                curnode->Len++;
            }break;
        }
    }
end:
    return res;
}

static inline tr_match_node_t* __tr_newnode(char _val)
{
    tr_match_node_t* res=0;
    res = &base->NodePool[base->NodePoolCount];
    base->NodePoolCount++;
    LIST_INIT(&res->BroList);
    LIST_INIT(&res->FSList);
    res->OP = _val;
    res->Content = NULL;
    res->Len = 0;

    return res;
}

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