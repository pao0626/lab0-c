#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list_sort.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)  // element_t{list}
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;
    char *copy = strdup(s);
    if (!copy) {
        free(element);
        return false;
    }
    element->value = copy;
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;
    char *copy = strdup(s);
    if (!copy) {
        free(element);
        return false;
    }
    element->value = copy;
    list_add_tail(&element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = list_first_entry(head, element_t, list);
    list_del(&element->list);  // not delete, is remove
    if (sp) {
        strncpy(sp, element->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = list_last_entry(head, element_t, list);
    list_del(&element->list);  // not delete, is remove
    if (sp) {
        strncpy(sp, element->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

void swap_values(element_t *a, element_t *b)
{
    char *temp = a->value;
    a->value = b->value;
    b->value = temp;
}

/* Shuffle every nodes in queue */
void q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    int len = q_size(head);
    struct list_head *old;
    struct list_head *new = head->prev;
    while (len != 1) {
        int idx = rand() % len;
        old = head->next;
        while (idx--) {
            old = old->next;
        }
        swap_values(list_entry(new, element_t, list),
                    list_entry(old, element_t, list));
        len--;
    }
    return;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *left = head->next;
    struct list_head *right = head->prev;
    while (left != right && left->next != right) {
        left = left->next;
        right = right->prev;
    }
    list_del(right);
    element_t *element = list_entry(right, element_t, list);
    q_release_element(element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    element_t *element, *safe;
    bool dup = false;
    list_for_each_entry_safe (element, safe, head, list) {
        if (&safe->list == head) {
            if (dup) {
                list_del(&element->list);
                q_release_element(element);
            }
            return true;
        }
        if (!strcmp(element->value, safe->value)) {
            list_del(&element->list);
            q_release_element(element);
            dup = true;
        } else {
            if (dup) {
                list_del(&element->list);
                q_release_element(element);
                dup = false;
            }
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head) {
            return;
        }
        list_move(node, node->next);
    }
    return;
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
    return;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head)
        return;
    int count = 0;
    struct list_head *node, *safe, *cut = head;
    LIST_HEAD(tmp_head);
    list_for_each_safe (node, safe, head) {
        count++;
        if (count == k) {
            list_cut_position(&tmp_head, cut, node);
            q_reverse(&tmp_head);
            list_splice_init(&tmp_head, cut);
            count = 0;
            cut = safe->prev;
        }
    }
    return;
}

/* merge two queue in ascending/descending order */
void q_merge_two(struct list_head *first,
                 struct list_head *second,
                 bool descend)
{
    if (!first || !second) {
        return;
    }
    LIST_HEAD(tmp);
    while (!list_empty(first) && !list_empty(second)) {
        element_t *first_elem = list_first_entry(first, element_t, list);
        element_t *second_elem = list_first_entry(second, element_t, list);
        if (descend) {
            if (strcmp(first_elem->value, second_elem->value) >= 0) {
                list_move_tail(first->next, &tmp);
            } else
                list_move_tail(second->next, &tmp);
        } else {
            if (strcmp(first_elem->value, second_elem->value) <= 0) {
                list_move_tail(first->next, &tmp);
            } else
                list_move_tail(second->next, &tmp);
        }
    }
    if (list_empty(second)) {
        list_splice_tail_init(first, &tmp);
    }
    list_splice_init(&tmp, second);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *left = head->next;
    struct list_head *right = head->prev;
    while (left != right && left->next != right) {
        left = left->next;
        right = right->prev;
    }
    LIST_HEAD(first);
    // first for left part of Linklist, left == mid
    list_cut_position(&first, head, left);
    q_sort(&first, descend);
    q_sort(head, descend);
    q_merge_two(&first, head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *cur = list_last_entry(head, element_t, list);
    int count = 1;
    while (cur->list.prev != head) {
        element_t *prev = list_entry(cur->list.prev, element_t, list);
        if (strcmp(prev->value, cur->value) <= 0) {
            count++;
            cur = prev;
        } else {
            list_del(&prev->list);
            q_release_element(prev);
        }
    }
    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *cur = list_entry(head->prev, element_t, list);
    int count = 1;
    while (cur->list.prev != head) {
        element_t *prev = list_entry(cur->list.prev, element_t, list);
        if (strcmp(prev->value, cur->value) >= 0) {
            count++;
            cur = prev;
        } else {
            list_del(&prev->list);
            q_release_element(prev);
        }
    }
    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);
    struct list_head *element;
    int ret = first->size;
    list_for_each (element, head) {
        if (list_entry(element, queue_contex_t, chain)->id == first->id)
            continue;
        ret += list_entry(element, queue_contex_t, chain)->size;
        q_merge_two(list_entry(element, queue_contex_t, chain)->q, first->q,
                    descend);
    }
    return ret;
}

int cmp_function(void *priv,
                 const struct list_head *a,
                 const struct list_head *b)
{
    element_t *a_entry = list_entry(a, element_t, list);
    element_t *b_entry = list_entry(b, element_t, list);
    bool descend = *(bool *) priv;  // 将 void* 参数转换回 bool
    if (descend) {
        return strcmp(a_entry->value, b_entry->value) < 0;
    } else {
        return strcmp(a_entry->value, b_entry->value) > 0;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_linux_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    list_sort(&descend, head, cmp_function);
}