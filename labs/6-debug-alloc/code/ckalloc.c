// implement a simple ckalloc/free that adds ckhdr_t to the 
// allocation.

#include "test-interface.h"
#include "ckalloc.h"

// keep a list of allocated blocks.
static hdr_t *alloc_list;
static hdr_t *free_list;

unsigned ck_verbose_p = 0;

// returns pointer to the first header block.
hdr_t *ck_first_hdr(void) {
    return alloc_list;
}

// returns pointer to next hdr or 0 if none.
hdr_t *ck_next_hdr(hdr_t *p) {
    if(p)
        return p->next;
    return 0;
}


void *ck_hdr_start(hdr_t *h) {
    return &h[1];
}

// one past the last byte of allocated memory.
void *ck_hdr_end(hdr_t *h) {
    return (char *)ck_hdr_start(h) + ck_nbytes(h);
}

// is ptr in <h>?
unsigned ck_ptr_in_block(hdr_t *h, void *ptr) {
    char *start = (char *)(h + 1);
    char *end = start + h->nbytes_alloc;
    char *p = (char *)ptr;
    return (p >= start && p < end);
}


hdr_t *ck_ptr_is_alloced(void *ptr) {
    for(hdr_t *h = ck_first_hdr(); h; h = ck_next_hdr(h))
        if(ck_ptr_in_block(h,ptr)) {
            // output("found %p in %p\n", ptr, h);
            return h;
        }
    return NULL;
}

static void list_remove(hdr_t **l, hdr_t *h) {
    assert(l);
    hdr_t *prev = *l;
 
    if(prev == h) {
        *l = h->next;
        return;
    }

    hdr_t *p;
    while((p = ck_next_hdr(prev))) {
        if(p == h) {
            prev->next = p->next;
            return;
        }
        prev = p;
    }
    panic("did not find %p in list\n", h);
}


void make_redzone(char *p, unsigned n) {
    memset(p, REDZONE_VAL, n);
}

int check_redzone(char *p, unsigned n) {
    for(unsigned i = 0; i < n; i++) {
        if(p[i] != REDZONE_VAL) {
            printk("found error at %p, %u, %d\n", p, i, p[i]);
            return 4*i + 1;
        }
    }
    return 0;
}

static int ckalloc_count = 1;

// free a block allocated with <ckalloc>
void (ckfree)(void *addr, src_loc_t l) {
    hdr_t *h = (void *)addr;
    h -= 1;

    int ofst = check_redzone(h->rz1, REDZONE_NBYTES);
    if (ofst != 0) {
        trace("ERROR:block %u  corrupted at offset %d\n",
              h->block_id, ofst - 1);
        trace("        logical block id=%u,  nbytes=%u\n", h->block_id, h->nbytes_alloc);
        trace("        Block allocated at: %s:%s:%d\n", h->alloc_loc.file, h->alloc_loc.func, h->alloc_loc.lineno);
        
        panic("corrupted block %u [addr=%p]", h->block_id, addr);
    }        
    
    ofst = check_redzone((char *)addr + h->nbytes_alloc, REDZONE_NBYTES);
    if (ofst != 0) {
        trace("ERROR:block %u  corrupted at offset %d\n",
              h->block_id, ofst - 1 + h->nbytes_alloc);
        trace("        logical block id=%u,  nbytes=%u\n", h->block_id, h->nbytes_alloc);
        trace("        Block allocated at: %s:%s:%d\n", h->alloc_loc.file, h->alloc_loc.func, h->alloc_loc.lineno);
        
        panic("corrupted block %u [addr=%p]", h->block_id, addr);
    }        

    

    if(h->state != ALLOCED)
        loc_panic(l, "freeing unallocated memory: state=%d\n", h->state);
    loc_debug(l, "freeing block=%u [addr=%p]\n", h->block_id, addr);
    
    h->state = FREED;
    make_redzone((char *)(h + 1), h->nbytes_alloc); // nuke data
    assert(ck_ptr_is_alloced(addr));

    list_remove(&alloc_list, h);
    h->next = free_list;
    free_list = h;


    // list_remove(&alloc_list, h);
    assert(!ck_ptr_is_alloced(addr));
}


// interpose on kr_malloc allocations and
//  1. allocate enough space for a header and fill it in.
//  2. add the allocated block to  the allocated list.
void *(ckalloc)(uint32_t nbytes, src_loc_t loc) {


    hdr_t *h = kr_malloc(nbytes + sizeof *h + REDZONE_NBYTES);

    memset(h, 0, sizeof *h);
    h->nbytes_alloc = nbytes;
    h->state = ALLOCED;
    h->alloc_loc = loc;
    h->block_id = ckalloc_count++;

    loc_debug(loc, "allocating block=%u [addr=%p]\n", h->block_id, h);

    void *addr = (void *)(h + 1);

    make_redzone(h->rz1, REDZONE_NBYTES);
    make_redzone((char *)addr + nbytes, REDZONE_NBYTES);

    assert(!ck_ptr_is_alloced(addr));
    h->next = alloc_list;
    alloc_list = h;

    assert(ck_ptr_is_alloced(addr));

    return addr;
}


int ck_heap_errors() {
    int errors = 0;
    int nblocks = 0;
    for(hdr_t *h = alloc_list; h; h = h->next) {
        nblocks++;

        if (h->state == ALLOCED) {
            if (ck_verbose_p) printk("alloc block block=%u [addr=[%p]\n", h->block_id, h);
            if (check_redzone(h->rz1, REDZONE_NBYTES) ||
                check_redzone((char *)(h + 1) + h->nbytes_alloc, REDZONE_NBYTES)) {
                    errors++;
            }
        } else {
            panic("free block %u [addr=%p] in alloc list", h->block_id, h);
        }
    }

    for(hdr_t *h = free_list; h; h = h->next) {
        nblocks++;

        if (h->state == FREED) {
            if (ck_verbose_p) printk("free block block=%u [addr=%p]\n", h->block_id, h);
            if (check_redzone(h->rz1, REDZONE_NBYTES) ||
                check_redzone((char *)(h + 1), h->nbytes_alloc) ||
                check_redzone((char *)(h + 1) + h->nbytes_alloc, REDZONE_NBYTES)) {
                    errors++;
            }
        } else {
            panic("alloc block %u [addr=%p] in free list", h->block_id, h);
        }

    }

    return errors;
}
