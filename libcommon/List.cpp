#include "List.h"

static ListNode * list_node_new(void *obj) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    node->obj = obj;
    node->last = NULL;
    node->next = NULL;
    return node;
}

static void list_node_free(ListNode *node, list_obj_free_cb free_cb) {
    node->next = NULL;
    node->last = NULL;
    if(free_cb) {
        free_cb(node->obj);
    }
    node->obj = NULL;
    free(node);
}


List * list_new() {
    List *list = (List *) malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}


void list_free(List *list, list_obj_free_cb free_cb) {
    if(list->head) {
        ListNode *cur = list->head;
        ListNode *next = NULL;
        while(cur) {
            next = cur->next;
            list_node_free(cur, free_cb);
            cur = next;
        }
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    free(list);
}


void list_push(List *list, void *obj) {
    if(!obj) {
        return ;
    }
    ListNode *node = list_node_new(obj);
    if(!(list->head)) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        node->last = list->tail;
        list->tail = node;
    }
    list->size++;
}

void* list_pop(List *list) {
    if(list->size <= 0 || !(list->tail)) {
        return NULL;
    }
    ListNode *tail = list->tail;
    void *obj = tail->obj;
    list->tail = tail->last;
    if(list->tail) {
        list->tail->next = NULL;
    }
    if(tail == list->head) {
        list->head = NULL;
    }
    list_node_free(tail, NULL);
    list->size--;
    return obj;
}


void list_shift(List *list, void *obj) {
    if(!obj) {
        return ;
    }
    ListNode *node = list_node_new(obj);
    if(!(list->head)) {
        list->head = node;
        list->tail = node;
    } else {
        list->head->last = node;
        node->next = list->head;
        list->head = node;
    }
    list->size++;
}

void* list_unshift(List *list) {
    if(list->size <= 0 || !(list->tail)) {
        return NULL;
    }
    ListNode *head = list->head;
    void *obj = head->obj;
    list->head = head->next;
    if(list->head) {
        list->head->last = NULL;
    }
    if(head == list->tail) {
        list->tail = NULL;
    }
    list_node_free(head, NULL);
    list->size--;
    return obj;
}

void list_foreach(List *list, list_foreach_cb fn) {
    ListNode *cur = list->head;
    while(cur) {
        fn(cur->obj);
        if(cur == list->tail) {
            break;
        }
        cur = cur->next;
    }
}

void* list_get(List *list, int index) {
    if(index < 0 || index >= list->size) {
        return NULL;
    }
    ListNode *cur = list->head;
    int off = 0;
    while(cur && off < list->size) {
        if(off == index) {
            return cur->obj;
        }
        cur = cur->next;
        off++;
    }
    return NULL;
}

size_t list_size(List *list) {
    return list->size;
}