//
// Created by root on 8/6/18.
//

#ifndef TRACER_LIST_H
#define TRACER_LIST_H


//A list head contains two pointers to list heads;
struct list_head_t {

	//Pointers to previous and next elements;
	struct list_head_t *prev, *next;

};



//Validity check for list add; Library internal;
void __list_add_valid(struct list_head_t *new, struct list_head_t *prev, struct list_head_t *next);


//Link the couple of list heads;
#define __list_link(p, n) (p)->next = (n), (n)->prev = (p);


//Add the new element between previous and next. Library internal, doesn't make check;
static inline void __list_add(struct list_head_t *new, struct list_head_t *prev, struct list_head_t *next) {

__list_link(prev, new);
__list_link(new, next);

}


/**
 * list_empty : asserts if the list's tail is empty;
 * @head : the list's head;
 */

static inline bool list_empty(struct list_head_t *head) {

	//Compare if head's successor is head.
	return head->next == head;

}


/**
 * list_add : concatenates the end of s0 and the head of l1
 */
static inline void list_add(struct list_head_t *src, struct list_head_t *dst) {

	//Cache ends of src and dst;
	struct list_head_t *src_e = src->prev;
	struct list_head_t *dst_e = dst->prev;

	//Link the end of dst to the head of src;
	__list_link(dst_e, src);

	//Link the end of src to the head of dst;
	__list_link(src_e, dst);

}


/**
 * list_remove : Remove the list head from the rest of the list; head will be linked to itself;
 *
 */
static inline void list_remove(struct list_head_t *l) {

	//First, cache the list neighbors;
	struct list_head_t *prev = l->prev, *next = l->next;

	//Link prev and next;
	__list_link(prev, next);

	//Link l with itself;
	__list_link(l, l);

}



#endif //TRACER_LIST_H
