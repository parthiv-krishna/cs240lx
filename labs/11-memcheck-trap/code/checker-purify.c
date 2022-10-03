// engler: cs240lx: a sort-of purify checker: gives an error
// message if a load/store to a heap addres is:
//   - not within a legal block.
//   - to freed memory.
//
// uses the checking allocator (ckalloc) and the kr-malloc
// implementation extended with a couple of new helper routines.
// [defined in ../../9-memcheck-stat/starter-code/ckalloc.h]
// 
// limits:
//   - does not check that its within the *correct*
//     legal block (can do this w/ replay).
//   - does not track anything about global or stack memory.
#include "memtrace.h"
#include "ckalloc.h"
#include "purify.h"

// heap management is janky; we do it
// in this ugly way so we don't redo
// old stuff and keep it all in one place.
static void * heap_start;
static void * heap_end;

static void * shadow_start;
static void * shadow_end;

#define SHADOWMEM_OFFSET (1024*512)
enum {
    ALLOC = 0xaa,
    FREE = 0xff
};

// is addresss <addr> in the heap?  if so can check.
// 
// [currently we only fault on the heap, so is just
// a sanity check.]
static inline 
int in_heap(uint32_t addr) {
    void *p = (void*)addr;
    if(p >= heap_start && p < heap_end)
        return 1;
    return 0;
}

// gross: we replicate this from ck-gc.c
//  - note: should only get called from our
//    routines so trapping will be off.
void *sbrk(long increment) {
    static int total;

    assert(increment > 0);
    if(total > SHADOWMEM_OFFSET)
        panic("allocated too much\n");
    total += increment;

    // if shadow memory: initialize.
    return kmalloc(increment);
}


void *get_shadow_loc(void *p) {
    char *result = (char*)shadow_start + (p - heap_start);
    // printk("loc: %p shadow loc: %p\n", p, result);
    return (void *)result;
}


// note: since we use pc don't really need to disable/enable.
void *purify_alloc_raw(unsigned n, src_loc_t l) {
    memtrace_trap_disable();

        // if shadow memory: mark [p,p+n) as allocated
        unsigned *p = (ckalloc)(n, l);
        // printk("alloc %d bytes at %p\n", n, p);
        memset(get_shadow_loc((void*)p), ALLOC, n);

    memtrace_trap_enable();
    return p;
}

void purify_free_raw(void *p, src_loc_t l) {
    memtrace_trap_disable();

        // if shadow memory: mark [p,p+n) as free
        (ckfree)(p, l);
        hdr_t *h = (hdr_t*)p - 1;
        memset(get_shadow_loc(p), FREE, ck_nbytes(h));
        // printk("free %d bytes at %p\n", ck_nbytes(h), p);

    memtrace_trap_enable();
}

// some error reporting: note that (as usual) reporting takes way 
// more code than checking!  [articulating "why" is more work than
// just detecting a violation]
//
// currently we just report an error and die.
//  - could try skipping the operation.
static void purify_error(uint32_t pc, void *addr, const char *op) {
    // illegal pointer: see if it is within a previously 
    // allocated block's redzones or header.
    hdr_t *h = ck_get_containing_blk(addr);
    if(!h)
        panic("ERROR: illegal %s to pointer [%x]:  not within a known block\n", 
                op, addr);

    int offset = ck_illegal_offset(h, addr);

    // regular use after free error
    if(offset == 0 && h->state == FREED) {
        trace("ERROR: use after free at [pc=%x]: illegal %s to [addr=%x] within freed block\n",
            pc, op, addr);
        hdr_print(h);
        clean_reboot();
    }

    const char *state = "allocated";
    if(h->state == FREED) 
        state = "FREED";
    const char *where = "after";
    if(offset < 0)
        where = "before";

    trace("ERROR: illegal %s to %s block at [pc=%x]: [addr=%x] is %d bytes %s legal mem (block size=%d)\n", 
            op,
            state,
            pc,
            addr,
            offset, 
            where, 
            ck_nbytes(h));
    hdr_print(h);
    clean_reboot();
}

// exception handler called by memtrace to handle a domain protection fault 
// caused by a load (<load_p>=1) or store (<load_p>=0) to <addr>.
//
//  - <regs> has the full set of registers.  you can modify these to 
//    change resumption behavior.  you'll have to disassemble the 
//    faulting instruction to get what registers it uses.
//
//  - note: called with trapping protection disabled.  if you want to 
//    resume directly will need to re-enable.
//
// returns:
//   - 0 to skip the memory instruction on resumption.  (e.g. can use for 
//     failure oblivious computing).
//   - 1 to execute the memory instruction without trapping.
//
static int purify_handler(uint32_t pc, uint32_t addr, unsigned load_p) {
    const char *op = load_p ? "load from" : "store to";

    // can be a lot of output.  for today, we leave it.
    trace("[pc=%x]: %s address %x\n", pc, op, addr);
    
    if(!in_heap(addr))
        panic("\t%x is not a heap addr: how are we faulting?\n", addr);

    char *shadow_addr = get_shadow_loc((void*) addr);
    if (shadow_addr > (char *)shadow_end) {
        return 0;
    }
    // printk("SHADOR %p: %x\n", shadow_addr, *shadow_addr);
    // was allocated and is legal.
    if(*shadow_addr == ALLOC)
        return 1;
    purify_error(pc, (void*)addr, op);
    return 0;
}

void purify_init(void) {
    // if you want to add shadow memory: just split heap 
    // in half (make sure you update in_heap).  
    //      - key: client can't corrupt b/c we'd trap.
    memtrace_init_default(purify_handler);

    // gross that this is hardcoded.
    shadow_start = kmalloc_heap_start();
    shadow_end = shadow_start + SHADOWMEM_OFFSET;
    heap_start = shadow_end;
    heap_end = heap_start + SHADOWMEM_OFFSET;
    // printk("heap mem: [%x,%x)\n", heap_start, heap_end);
    // printk("shadow mem: [%x,%x)\n", shadow_start, shadow_end);
    memtrace_trap_disable();
    memset(shadow_start, FREE, SHADOWMEM_OFFSET);
    memtrace_trap_enable();

    memtrace_on();
}
