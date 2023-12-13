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

/* Define to 1 if there DON'T want '.' to match '\r' + '\n' */
#ifndef TINY_REGEX_CONFIG_DOT_IGNORE_NEWLINE
#define TINY_REGEX_CONFIG_DOT_IGNORE_NEWLINE   1
#endif

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

typedef struct {
    char* Buf;
    uint32_t Len;
}tr_re_t;






#ifdef __cplusplus
}
#endif
#endif
