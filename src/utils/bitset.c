#include "bitset.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BitSet *new_bitset(int size) {
    BitSet *bs = (BitSet *)malloc(sizeof(BitSet));
    bs->size = size;
    bs->n_words = (int)(size + BITS_PER_WORD - 1) / BITS_PER_WORD;
    bs->words = (word_t *)calloc(bs->n_words, sizeof(word_t));
    return bs;
}

BitSet *bitset_copy(BitSet *bs) {
    if (bs == NULL)
        return NULL;

    BitSet *new_bs = new_bitset(bs->size);
    memcpy(new_bs->words, bs->words, bs->n_words * sizeof(word_t));
    return new_bs;
}

void bitset_set(BitSet *bs, int pos) {
    if (pos >= bs->size)
        return;

    int word_idx = pos / BITS_PER_WORD;
    int bit_idx = pos % BITS_PER_WORD;
    bs->words[word_idx] |= (1UL << bit_idx);
}

void bitset_clear(BitSet *bs, int pos) {
    if (pos >= bs->size)
        return;

    int word_idx = pos / BITS_PER_WORD;
    int bit_idx = pos % BITS_PER_WORD;
    bs->words[word_idx] &= ~(1UL << bit_idx);
}

void bitset_toggle(BitSet *bs, int pos) {
    if (pos >= bs->size)
        return;

    int word_idx = pos / BITS_PER_WORD;
    int bit_idx = pos % BITS_PER_WORD;
    bs->words[word_idx] ^= (1UL << bit_idx);
}

bool bitset_test(const BitSet *bs, int pos) {
    if (bs == NULL)
        return false;

    if (pos >= bs->size)
        return false;

    int word_idx = pos / BITS_PER_WORD;
    int bit_idx = pos % BITS_PER_WORD;
    return (bs->words[word_idx] & (1UL << bit_idx)) != 0;
}

void bitset_union(BitSet *dest, const BitSet *src) {
    if (src == NULL)
        return;

    int n_words = min(dest->n_words, src->n_words);
    for (int i = 0; i < n_words; i++) {
        dest->words[i] |= src->words[i];
    }
}

void bitset_diff(BitSet *dest, const BitSet *src) {
    if (dest == NULL || src == NULL)
        return;

    int n_words = min(dest->n_words, src->n_words);
    for (int i = 0; i < n_words; i++) {
        dest->words[i] &= ~src->words[i];
    }
}

bool bitset_equals(const BitSet *bs1, const BitSet *bs2) {
    if (bs1 == NULL && bs2 == NULL)
        return true;

    if (bs1 == NULL || bs2 == NULL)
        return false;

    return memcmp(bs1->words, bs2->words, bs1->n_words * sizeof(word_t)) == 0;
}

void print_bitset(const BitSet *bs) {
    if (bs == NULL) {
        printf("\xe2\x88\x85\n");
        return;
    }

    for (int i = 0; i < bs->size; i++) {
        printf("%d", bitset_test(bs, i) ? 1 : 0);
        if ((i + 1) % 8 == 0 && i + 1 < bs->size) {
            printf(" ");
        }
    }
    printf("\n");
}

void destroy_biset(BitSet *bs) {
    if (bs == NULL)
        return;

    free(bs->words);
    free(bs);
}
