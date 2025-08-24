#ifndef BITSET_H
#define BITSET_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t word_t;
#define BITS_PER_WORD (sizeof(word_t) * CHAR_BIT)

typedef struct BitSet {
    word_t *words;
    int size;
    int n_words;
} BitSet;

/**
 * @brief Creates a new BitSet.
 * @param size The number of bits (virtual registers) to accommodate.
 * @return A pointer to the new BitSet.
 */
BitSet *new_bitset(int size);
/**
 * @brief Creates a copy of a BitSet.
 */
BitSet *bitset_copy(BitSet *bs);
/**
 * @brief Adds an element to the BitSet.
 */
void bitset_set(BitSet *bs, int pos);
/**
 * @brief Remove an element to the BitSet.
 */
void bitset_clear(BitSet *bs, int pos);
/**
 * @brief Toggle an element to the BitSet.
 */
void bitset_toggle(BitSet *bs, int pos);
/**
 * @brief Checks if a member is in the BitSet.
 */
bool bitset_test(const BitSet *bs, int pos);
/**
 * @brief Performs the union of two BitSets (dest = dest U src).
 */
void bitset_union(BitSet *dest, const BitSet *src);
/**
 * @brief Performs the difference of two BitSets (dest = dest - src).
 */
void bitset_diff(BitSet *dest, const BitSet *src);
/**
 * @brief Checks if two BitSets are equal.
 */
bool bitset_equals(const BitSet *bs1, const BitSet *bs2);
/**
 * @brief Prints the BitSet as binary string
 */
void print_bitset(const BitSet *bs1);
/**
 * @brief Destroy a BitSet and free its memory.
 */
void destroy_biset(BitSet *bs);

#endif // BITSET_H
