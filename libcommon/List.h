#pragma once

#include <stdlib.h>

typedef struct ListNode_st {
    void                  *obj;
    struct ListNode_st    *next;
    struct ListNode_st    *last;
} ListNode;

typedef struct List_st {
    ListNode              *head;
    ListNode              *tail;
    size_t                 size;
} List;

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*list_foreach_cb)(void *obj);
typedef void (*list_obj_free_cb)(void *obj);

List * list_new();
void list_free(List *list, list_obj_free_cb free_cb);
void list_push(List *list, void *obj);
void* list_pop(List *list);
void list_shift(List *list, void *obj);
void* list_unshift(List *list);
void list_foreach(List *list, list_foreach_cb fn);
void* list_get(List *list, int index);
size_t list_size(List *list);

#ifdef __cplusplus
}
#endif


