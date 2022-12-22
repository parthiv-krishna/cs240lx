#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "smt.h"

// Keep track of clauses to be sent to the SAT solver
struct clause {
    int *literals, n_literals;
};
static struct clause *CLAUSES = NULL;
static int N_CLAUSES = 0;

void clause_arr(int *literals) {
    int i = N_CLAUSES;
    APPEND_GLOBAL(CLAUSES) = (struct clause){
        .literals = NULL,
        .n_literals = 0,
    };
    for (; *literals; literals++) {
        APPEND_FIELD(CLAUSES[i], literals) = *literals;
    }
}

// Keep track of arrays in a tree structure. (store a k v) creates a new array
// whose parent is a, bv_store_key is k, and bv_store_value is v. Whenever a
// get is done on the array, a corresponding bv_lookup is appended to its list.
struct bv_lookup {
    int bv_key, bv_value;
};

struct array {
    int array_parent;

    int bv_store_key, bv_store_value;

    struct bv_lookup *bv_lookups;
    int n_bv_lookups;
};
static struct array *ARRAYS = NULL;
static int N_ARRAYS = 0;

// Keep track of bitvectors
struct bv {
    int *bits;
    int n_bits;
};
static struct bv *BVS = NULL;
static int N_BVS = 0;

// You can get a fresh SAT variable like NEXT_SAT_VAR++
static int NEXT_SAT_VAR = 1;

int new_array() {
    // Create a new array. It has no parent (-1) and no lookups yet (NULL, 0).
    // Returns its index in the ARRAYS vector.
    assert(!"Implement me!");
}

int array_store(int old_array, int bv_key, int bv_value) {
    // Construct a new array that is the same as old_array except bv_key is
    // updated to bv_value. This should look like new_array() except you set
    // array_parent, bv_store_key, bv_store_value correctly.
    assert(!"Implement me!");
}

int array_get(int array, int bv_key, int out_width) {
    // Create a new bitvector and (on the corresponding array) record this
    // key/value pair lookup.
    assert(!"Implement me!");
}

int new_bv(int width) {
    // Create a fresh bitvector of the given width. Note here you need to
    // append to the BVS vector as well as initialize all its bits to fresh SAT
    // variables.
    struct bv b;
    memset(&b, 0, sizeof(struct bv));
    for (int i = 0; i < width; i++) {
        APPEND_FIELD(b, bits) = NEXT_SAT_VAR++;
    }
    APPEND_GLOBAL(BVS) = b;
    return N_BVS - 1;
}

int const_bv(int64_t value, int width) {
    // Like new_bv, except also add clauses asserting its bits are the same as
    // those of @value. Please do little-endian; bits[0] should be value & 1.
    int bv_idx = new_bv(width);
    struct bv b = BVS[bv_idx];
    for (int i = 0; i < b.n_bits; i++) {
        if ((value >> i) & 1) {
            clause(b.bits[i]);
        } else {
            clause(-b.bits[i]);
        }
    }
}

int bv_eq(int bv_1, int bv_2) {
    int width = BVS[bv_1].n_bits;
    assert(width == BVS[bv_2].n_bits);
    int final_clause_arr[width + 2];

    // This one is a doozy: add a fresh SAT variable and enough SAT clauses to
    // assert that this variable is true iff all the bits of bv_1 and bv_2 are
    // equal. I suggest using as many of intermediate/temp SAT variables as you
    // need; generally, BCP does a great job at handling those so they're more
    // or less "free" (at least, for our purposes rn!).
    int sat_final = NEXT_SAT_VAR++;
    for (int i = 0; i < width; i++) {
        int sat_1 = BVS[bv_1].bits[i];
        int sat_2 = BVS[bv_2].bits[i];
        int sat_intermediate = NEXT_SAT_VAR++;
        clause(sat_1, sat_2, sat_intermediate); // (~1 and ~2) -> intermediate
        clause(-sat_1, -sat_2, sat_intermediate); // (1 and 2) -> intermediate
        clause(-sat_intermediate, -sat_1, sat_2); // intermediate -> (~1 and 2)
        clause(-sat_intermediate, sat_1, -sat_2); // intermediate -> (1 and ~2)
        clause(-sat_final, sat_intermediate); // final->intermediate

        final_clause_arr[i] = -sat_intermediate;
    }
    final_clause_arr[width] = sat_final; // (i1 and i2 and ... iN) -> final
    final_clause_arr[width + 1] = 0; // terminate
    clause_arr(final_clause_arr);
    return sat_final;
}

int bv_add(int bv_1, int bv_2) {
    int width = BVS[bv_1].n_bits;
    assert(width == BVS[bv_2].n_bits);

    int out = new_bv(width);

    // This one is similarly big; basically you want to do a ripple-carry
    // adder to assign the bits of @out based on the bits of @bv_1, @bv_2. I
    // would recommend just trying to encode it as a truth table.
    int carry[width];
    for (int i = 0; i < width; i++) {
        carry[i] = NEXT_SAT_VAR++;
    }
    
    clause(-carry[0]);
    for (int i = 0; i < width; i++) {
        // 


    }
    return out;
}

static void array_axioms(struct array array, int compare_up_to,
                         struct bv_lookup bv_lookup, int already_handled) {
    if (array.array_parent >= 0) {
        // This array is of the form (store parent k v), where k and v are
        // array.bv_store_*. Add clauses here of the form:
        // ((k == lookup.key) ^ !already_handled) => (store_val == bv_val)
        // Hint: use bv_eq!
        assert(!"Implement me!");

        // Now we need to record if that worked or not. If this is picking up
        // that store, then we don't need the value to match with any prior
        // values (because they were overridden). So make a fresh SAT variable
        // new_already_handled and assign it:
        // (already_handled or (k == lookup.key)) <=> new_already_handled
        assert(!"Implement me!");

        // Then update already_handled = new_already_handled
        assert(!"Implement me!");
    }

    // If we haven't found a set yet, compare lookup[key] to all prior lookups
    // on the array. If keys eq, vals should be eq too.
    for (int i = 0; i < compare_up_to; i++) {
        struct bv_lookup sub_lookup = array.bv_lookups[i];
        // We want:
        // !already_handled ^ (sub_lookup.key == bv_lookup.key)
        // => sub_lookup.value == bv_lookup.value
        assert(!"Implement me!");
    }

    // If we haven't found a set yet, go back through parents and repeat this
    // reasoning.
    if (array.array_parent >= 0) {
        array_axioms(ARRAYS[array.array_parent],
                     ARRAYS[array.array_parent].n_bv_lookups,
                     bv_lookup, already_handled);
    }
}

int *SAT_SOLUTION = NULL;
int solve() {
    assert(!SAT_SOLUTION);
    char constraint_path[256] = "";
    sprintf(constraint_path, "temp_files/constraints.%d.dimacs", getpid());

    FILE *fout = fopen(constraint_path, "w");
    if (!fout) {
        fprintf(stderr, "Cannot create file %s\n", constraint_path);
        exit(1);
    }

    int zero = NEXT_SAT_VAR++;
    clause(-zero);

    // Go through array stores & lookups and set up the N^2 bv_eq implications
    // implied
    for (int i = 0; i < N_ARRAYS; i++) {
        for (int j = 0; j < ARRAYS[i].n_bv_lookups; j++) {
            array_axioms(ARRAYS[i], j, ARRAYS[i].bv_lookups[j], zero);
        }
    }

    // Print the clauses and some info about the variable interpretations
    for (int i = 0; i < N_BVS; i++) {
        fprintf(fout, "c bitvector %d:", i);
        for (int j = 0; j < BVS[i].n_bits; j++)
            fprintf(fout, " %d", BVS[i].bits[j]);
        fprintf(fout, "\n");
    }
    fprintf(fout, "p cnf %d %d\n", NEXT_SAT_VAR - 1, N_CLAUSES);

    // Iterate through the clauses and print them in DIMACS format.
    for (int i = 0; i < N_CLAUSES; i++) {
        struct clause c = CLAUSES[i];
        for (int j = 0; j < c.n_literals; j++) {
            int lit = c.literals[j];
            fprintf(fout, "%d ", lit);
        }
        fprintf(fout, "0\n");
    }

    fclose(fout);

    char result_path[256] = "";
    sprintf(result_path, "temp_files/results.%d.txt", getpid());

    char cmd[1024] = "";
#ifdef USE_MINISAT
    sprintf(cmd, "minisat %s %s > /dev/null", constraint_path, result_path);
#else
    sprintf(cmd, "cat %s | ./fast > %s", constraint_path, result_path);
#endif
    assert(system(cmd) != -1);

    FILE *results = fopen(result_path, "r");
    char sat_or_unsat[10] = "NONE";
    assert(fscanf(results, "%s ", sat_or_unsat));
    if (!strcmp(sat_or_unsat, "UNSAT")) {
        fclose(results);
        return 0;
    } else if (!strcmp(sat_or_unsat, "SAT")) {
        SAT_SOLUTION = malloc(NEXT_SAT_VAR * sizeof(SAT_SOLUTION[0]));
        int literal = 0;
        while (!feof(results) && fscanf(results, " %d ", &literal)) {
            if (literal < 0)    SAT_SOLUTION[-literal] = 0;
            else                SAT_SOLUTION[literal] = 1;
        }
        fclose(results);
        return 1;
    }
    exit(1);
}

int64_t get_solution(int bv, int as_signed) {
    assert(SAT_SOLUTION);

    // Read the bits for @bv from SAT_SOLUTION into an int64_t.
    int64_t result;
    struct bv b = BVS[bv];
    assert(bv < N_BVS);
    assert(b.n_bits < 64);
    for (int i = 0; i < b.n_bits; i++) {
        int sat_variable = b.bits[i];
        result |= (SAT_SOLUTION[sat_variable] << i);
    }
    return result;
}
