// written by jimpo, 5/19/20
#include "rpi.h"
#include "ckalloc.h"

struct list_node {
    struct list_node *next;
    int val;
};

struct list_node *l;

void notmain(void) {

    printk("GC test7\n");

    for (int i = 0; i < 10; i++) {
        struct list_node *n = (struct list_node *)ckalloc(sizeof(struct list_node));
        n->next = l;
        l = n;
    }

    if (ck_find_leaks(1)) {
        panic("should have no leaks!\n");
    } else {
        trace("SUCCESS: no leaks!\n");
    }

    printk("moving l\n");

    l = (struct list_node *)&(l->val);

    if (ck_find_leaks(0)) {
        panic("should have no leaks!\n");
    } else {
        trace("SUCCESS: no leaks!\n");
    }

    if (ck_find_leaks(1)) {
        trace("SUCCESS: found maybe leak!\n");
    } else {
        panic("should have maybe leak!\n");
    }


    printk("restoring l\n");
    l = (struct list_node *)&((l - 1)->val);

    if (ck_find_leaks(1)) {
        panic("should have no leaks!\n");
    } else {
        trace("SUCCESS: no leaks!\n");
    }

    printk("leaking l\n");
    l = (struct list_node *)0;

    if (ck_find_leaks(0)) {
        trace("SUCCESS: found leak!\n");
    } else {
        panic("should have maybe leak!\n");
    }

    printk("garbage collecting\n");
    ck_gc();


    if (ck_find_leaks(1)) {
        panic("should have no leaks!\n");
    } else {
        trace("SUCCESS: no leaks!\n");
    }
}
