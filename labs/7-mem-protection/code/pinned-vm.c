// put your code here.
//
#include "rpi.h"
#include "libc/bit-support.h"

// has useful enums and helpers.
#include "vector-base.h"
#include "pinned-vm.h"
#include "pinned-vm-asm.h"
#include "mmu.h"
#include "procmap.h"
#include "armv6-debug.h"

#define TLB_N_LOCKDOWN 8

// generate the _get and _set methods.
// (see asm-helpers.h for the cp_asm macro 
// definition)
// arm1176.pdf: 3-149

coproc_mk(tlb_lockdown_index, p15, 5, c15, c4, 2);
coproc_mk(tlb_lockdown_va, p15, 5, c15, c5, 2);
coproc_mk(tlb_lockdown_attr, p15, 5, c15, c7, 2);
coproc_mk(tlb_lockdown_pa, p15, 5, c15, c6, 2);



void lockdown_index_set(uint32_t x) {
    cp15_tlb_lockdown_index_set(x);
}
uint32_t lockdown_index_get(void) {
    return cp15_tlb_lockdown_index_get();
}

void lockdown_va_set(uint32_t x) {
    cp15_tlb_lockdown_va_set(x);
}
uint32_t lockdown_va_get(void) {
    return cp15_tlb_lockdown_va_get();
}

void lockdown_pa_set(uint32_t x) {
    cp15_tlb_lockdown_pa_set(x);
}
uint32_t lockdown_pa_get(void) {
    return cp15_tlb_lockdown_pa_get();
}

void lockdown_attr_set(uint32_t x) {
    cp15_tlb_lockdown_attr_set(x);
}
uint32_t lockdown_attr_get(void) {
    return cp15_tlb_lockdown_attr_get();
}


// do a manual translation in tlb:
//   1. store result in <result>
//   2. return 1 if entry exists, 0 otherwise.
int tlb_contains_va(uint32_t *result, uint32_t va) {

    // 3-79
    assert(bits_get(va, 0,2) == 0);

    // table 3-150 size field
    static unsigned split_opts[] = {24, 12, 16, 20};

    for (int i = 0; i < TLB_N_LOCKDOWN; i++) {

        // figure out where to split pa|va using page size
        lockdown_index_set(i);
        uint32_t pa_reg = lockdown_pa_get();
        uint32_t size = bits_get(pa_reg, 6, 7);
        unsigned split = split_opts[size];

        // compare msbs of given va with what the TLB has at this idx
        uint32_t va_msbs = bits_get(va, split, 31);
        uint32_t va_reg = lockdown_va_get();
        uint32_t va_reg_msbs = bits_get(va_reg, split, 31);
        if (va_msbs == va_reg_msbs) {
            // match. get the pa msbs
            uint32_t pa_reg_msbs = bits_get(pa_reg, split, 31);

            // construct pa, lsbs from va and msbs from tlb
            uint32_t pa = 0;
            pa = bits_set(pa, 0, split - 1, bits_get(va, 0, split - 1));
            pa = bits_set(pa, split, 31, pa_reg_msbs);
            *result = pa;
            return 1;
        }
    }
    return 0;
}

// map <va>-><pa> at TLB index <idx> with attributes <e>
void pin_mmu_sec(unsigned idx,  
                uint32_t va, 
                uint32_t pa,
                pin_t e) {

    demand(idx < 8, lockdown index too large);
    // lower 20 bits should be 0.
    demand(bits_get(va, 0, 19) == 0, only handling 1MB sections);
    demand(bits_get(pa, 0, 19) == 0, only handling 1MB sections);

    if(va != pa)
        panic("for today's lab, va (%x) should equal pa (%x)\n",
                va,pa);

    debug("about to map %x->%x\n", va,pa);


    // these will hold the values you assign for the tlb entries.
    uint32_t x, va_ent, pa_ent, attr;

    // put your code here.
    x = bits_set(0, 0, 2, idx);
    lockdown_index_set(x);

    // table 3-149
    va_ent = bits_set(0, 12, 31, bits_get(va, 12, 31));
    if (e.G) {
        va_ent = bit_set(va_ent, 9); // makes it global
    }
    va_ent = bits_set(va_ent, 0, 7, e.asid);
    lockdown_va_set(va_ent);

    // table 3-150
    pa_ent = bits_set(0, 12, 31, bits_get(pa, 12, 31)); 
    pa_ent = bit_set(pa_ent, 9); // not secure
    pa_ent = bits_set(pa_ent, 6, 7, e.pagesize); // size
    pa_ent = bits_set(pa_ent, 1, 3, e.AP_perm); // APX and AP
    pa_ent = bit_set(pa_ent, 0); // valid
    lockdown_pa_set(pa_ent);

    // table 3-152
    attr = bits_set(0, 7, 10, e.dom); // domain
    attr = bits_set(attr, 1, 5, e.mem_attr); // TEX C V
    lockdown_attr_set(attr);


    if((x = lockdown_va_get()) != va_ent)
        panic("lockdown va: expected %x, have %x\n", va_ent,x);
    if((x = lockdown_pa_get()) != pa_ent)
        panic("lockdown pa: expected %x, have %x\n", pa_ent,x);
    if((x = lockdown_attr_get()) != attr)
        panic("lockdown attr: expected %x, have %x\n", attr,x);
}


// check that <va> is pinned.  
void pin_check_exists(uint32_t va) {
    if(!mmu_is_enabled())
        panic("XXX: i think we can only check existence w/ mmu enabled\n");

    uint32_t r;
    if(tlb_contains_va(&r, va)) {
        pin_debug("success: TLB contains %x, returned %x\n", va, r);
        assert(va == r);
    } else
        panic("TLB should have %x: returned %x [reason=%b]\n", 
            va, r, bits_get(r,1,6));
}

// TLB pin all entries in procmap <p>
// very simpleminded atm.
void pin_procmap(procmap_t *p) {
    for(unsigned i = 0; i < p->n; i++) {
        pr_ent_t *e = &p->map[i];
        assert(e->nbytes == MB);

        switch(e->type) {
        case MEM_DEVICE:
                pin_mmu_sec(i, e->addr, e->addr, pin_mk_device(e->dom));
                break;
        case MEM_RW:
        {
                // currently everything is uncached.
                pin_t g = pin_mk_global(e->dom, perm_rw_priv, MEM_uncached);
                pin_mmu_sec(i, e->addr, e->addr, g);
                break;
        }
        case MEM_RO: panic("not handling\n");
        default: panic("unknown type: %d\n", e->type);
        }
    }
}


// turn the pinned MMU system on.
//    1. initialize the MMU (maybe not actually needed): clear TLB, caches
//       etc.  if you're obsessed with low line count this might not actually
//       be needed, but we don't risk it.
//    2. allocate a 2^14 aligned, 0-filled 4k page table so that any nonTLB
//       access gets a fault.
//    3. set the domain privileges (to DOM_client)
//    4. set the exception handler up using <vector_base_set>
//    5. turn the MMU on --- this can be much simpler than the normal
//       mmu procedure since it's never been on yet and we do not turn 
//       it off.
//    6. profit!
void pin_mmu_on(procmap_t *p) {
    assert(!mmu_is_enabled());

    // we have to clear the MMU before setting any entries.
    staff_mmu_init();
    pin_procmap(p);

    // 0 filled page table to get fault on any lookup.
    void *null_pt = kmalloc_aligned(4096 * 4, 1 << 14);
    demand((unsigned)null_pt % (1<<14) == 0, must be 14 bit aligned);

    // XXX: invalidate TLB routines don't seem to invalidate the 
    // pinned entries.

    // right now we just have a single domain?
    staff_domain_access_ctrl_set(DOM_client << kern_dom*2);

    // install the default vectors.
    extern uint32_t default_vec_ints[];
    vector_base_set(default_vec_ints);

    pin_debug("about to turn on mmu\n");
    staff_mmu_on_first_time(1, null_pt);
    assert(mmu_is_enabled());
    pin_debug("enabled!\n");

    // can only check this after MMU is on.
    pin_debug("going to check entries are pinned\n");
    for(unsigned i = 0; i < p->n; i++)
        pin_check_exists(p->map[i].addr);
}
