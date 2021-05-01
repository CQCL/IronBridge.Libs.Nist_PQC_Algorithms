/*! @file picnic2_impl.c
 *  @brief This is the main file of the signature scheme for the Picnic2
 *  parameter sets.
 *
 *  This file is part of the reference implementation of the Picnic signature scheme.
 *  See the accompanying documentation for complete details.
 *
 *  The code is provided under the MIT license, see LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "picnic_impl.h"
#include "picnic2_impl.h"
#include "picnic.h"
#include "platform.h"
#include "lowmc_constants.h"
#include "picnic_types.h"
#include "hash.h"
#include "tree.h"

#define MAX_AUX_BYTES ((LOWMC_MAX_AND_GATES + LOWMC_MAX_KEY_BITS) / 8 + 1)

/* Number of leading zeroes of x.
 * From the book
 * H.S. Warren, *Hacker's Delight*, Pearson Education, 2003.
 * http://www.hackersdelight.org/hdcodetxt/nlz.c.txt
 */
static int32_t nlz(uint32_t x)
{
    uint32_t n;

    if (x == 0) return (32);
    n = 1;
    if((x >> 16) == 0) {n = n + 16; x = x << 16;}
    if((x >> 24) == 0) {n = n + 8;  x = x << 8;}
    if((x >> 28) == 0) {n = n + 4;  x = x << 4;}
    if((x >> 30) == 0) {n = n + 2;  x = x << 2;}
    n = n - (x >> 31);

    return n;
}

uint32_t ceil_log2(uint32_t x)
{
    if (x == 0) {
        return 0;
    }
    return 32 - nlz(x - 1);
}

static void createRandomTapes(randomTape_t* tapes, uint8_t** seeds, uint8_t* salt, size_t t, paramset_t* params)
{
    HashInstance ctx;

    size_t tapeSizeBytes = 2 * params->andSizeBytes + params->stateSizeBytes;

    allocateRandomTape(tapes, params);
    for (size_t i = 0; i < params->numMPCParties; i++) {
        HashInit(&ctx, params, HASH_PREFIX_NONE);
        HashUpdate(&ctx, seeds[i], params->seedSizeBytes);
        HashUpdate(&ctx, salt, params->saltSizeBytes);
        HashUpdateIntLE(&ctx, t);
        HashUpdateIntLE(&ctx, i);
        HashFinal(&ctx);

        HashSqueeze(&ctx, tapes->tape[i], tapeSizeBytes);
    }
}

static uint64_t tapesToWord(randomTape_t* tapes)
{
    uint64_t shares;

    for (size_t i = 0; i < 64; i++) {
        uint8_t bit = getBit(tapes->tape[i], tapes->pos);
        setBit((uint8_t*)&shares, i, bit);
    }
    tapes->pos++;
    return shares;
}

/* Read one bit from each tape and assemble them into a word.  The tapes form a
 * z by N matrix, we'll transpose it, then the first "count" N-bit rows forms
 * an output word.  In the current implementation N is 64 so the words are
 * uint64_t. The return value must be freed with freeShares().
 */
static void tapesToWords(shares_t* shares, randomTape_t* tapes)
{
    for (size_t w = 0; w < shares->numWords; w++) {
        shares->shares[w] = tapesToWord(tapes);
    }
}

static void copyShares(shares_t* dst, shares_t* src)
{
    assert(dst->numWords == src->numWords);
    memcpy(dst->shares, src->shares, dst->numWords * sizeof(dst->shares[0]));
}

/* For an input bit b = 0 or 1, return the word of all b bits, i.e.,
 * extend(1) = 0xFFFFFFFFFFFFFFFF
 * extend(0) = 0x0000000000000000
 * Assumes inputs are always 0 or 1.  If this doesn't hold, add "& 1" to the
 * input.
 */
static uint64_t extend(uint8_t bit)
{
    return ~(bit - 1);
}

static uint64_t parity64(uint64_t x)
{
    uint64_t y = x ^ (x >> 1);

    y ^= (y >> 2);
    y ^= (y >> 4);
    y ^= (y >> 8);
    y ^= (y >> 16);
    y ^= (y >> 32);
    return y & 1;
}

static uint64_t aux_mpc_AND(uint64_t a, uint64_t b, randomTape_t* tapes, paramset_t* params)
{
    uint64_t mask_a = parity64(a);
    uint64_t mask_b = parity64(b);
    uint64_t fresh_output_mask = tapesToWord(tapes);

    uint64_t and_helper = tapesToWord(tapes);

    /* Zero the last party's share of the helper value, compute it based on the
     * input masks; then update the tape. */
    setBit((uint8_t*)&and_helper, params->numMPCParties - 1, 0);
    uint64_t aux_bit = (mask_a & mask_b) ^ parity64(and_helper);
    size_t lastParty = tapes->nTapes - 1;
    setBit(tapes->tape[lastParty], tapes->pos - 1, (uint8_t)aux_bit);

    return fresh_output_mask;
}

static void aux_mpc_sbox(shares_t* state, randomTape_t* tapes, paramset_t* params)
{
    for (size_t i = 0; i < params->numSboxes * 3; i += 3) {
        uint64_t a = state->shares[i + 2];
        uint64_t b = state->shares[i + 1];
        uint64_t c = state->shares[i];

        uint64_t ab = aux_mpc_AND(a, b, tapes, params);
        uint64_t bc = aux_mpc_AND(b, c, tapes, params);
        uint64_t ca = aux_mpc_AND(c, a, tapes, params);

        state->shares[i + 2] = a ^ bc;
        state->shares[i + 1] = a ^ b ^ ca;
        state->shares[i] = a ^ b ^ c ^ ab;
    }
}

static void mpc_xor_masks(shares_t* out, const shares_t* a, const shares_t* b)
{
    assert(out->numWords == a->numWords && a->numWords == b->numWords);

    for (size_t i = 0; i < out->numWords; i++) {
        out->shares[i] = a->shares[i] ^ b->shares[i];
    }
}

static void aux_matrix_mul(shares_t* output, const shares_t* vec, const uint32_t* matrix, shares_t* tmp_output, paramset_t* params)
{
    for (size_t i = 0; i < params->stateSizeBits; i++) {
        uint64_t new_mask_i = 0;
        for (uint32_t j = 0; j < params->stateSizeBits / 8; j++) {
            uint8_t matrix_byte = ((uint8_t*)matrix)[i * (params->stateSizeBits / 8) + j];
            new_mask_i ^= vec->shares[j * 8] & extend((matrix_byte >> 7) & 1);
            new_mask_i ^= vec->shares[j * 8 + 1] & extend((matrix_byte >> 6) & 1);
            new_mask_i ^= vec->shares[j * 8 + 2] & extend((matrix_byte >> 5) & 1);
            new_mask_i ^= vec->shares[j * 8 + 3] & extend((matrix_byte >> 4) & 1);
            new_mask_i ^= vec->shares[j * 8 + 4] & extend((matrix_byte >> 3) & 1);
            new_mask_i ^= vec->shares[j * 8 + 5] & extend((matrix_byte >> 2) & 1);
            new_mask_i ^= vec->shares[j * 8 + 6] & extend((matrix_byte >> 1) & 1);
            new_mask_i ^= vec->shares[j * 8 + 7] & extend(matrix_byte & 1);
        }
        tmp_output->shares[i] = new_mask_i;
    }

    copyShares(output, tmp_output);
}

#if 0
/* Simpler version of aux_matrix_mul, closer to the description in the spec */
static void aux_matrix_mul_simple(shares_t* output, const shares_t* vec, const uint32_t* matrix, shares_t* tmp_output, paramset_t* params)
{
    for (size_t i = 0; i < params->stateSizeBits; i++) {

        uint64_t new_mask_i = 0;
        for (uint32_t j = 0; j < params->stateSizeBits; j++) {
            uint8_t matrix_bit = getBit((uint8_t*)matrix, i * params->stateSizeBits + j);
            new_mask_i ^= vec->shares[j] & extend(matrix_bit);
        }
        tmp_output->shares[i] = new_mask_i;
    }

    copyShares(output, tmp_output);
}
#endif


/* Input is the tapes for one parallel repitition; i.e., tapes[t]
 * Updates the random tapes of all players with the mask values for the output of
 * AND gates, and computes the N-th party's share such that the AND gate invariant
 * holds on the mask values.
 */
static void computeAuxTape(randomTape_t* tapes, paramset_t* params)
{
    shares_t* roundKey = allocateShares(params->stateSizeBits);
    shares_t* state = allocateShares(params->stateSizeBits);
    shares_t* key = allocateShares(params->stateSizeBits);
    shares_t* tmp1 = allocateShares(params->stateSizeBits);

    tapesToWords(key, tapes);

    // The next line is the combination of two operations, simplified because XORs by
    // a constant is a NOP during preprocssing.
    // roundKey = key * KMatrix[0]
    // state = roundKey + plaintext
    aux_matrix_mul(state, key, KMatrix(0, params), tmp1, params);

    for (uint32_t r = 1; r <= params->numRounds; r++) {
        aux_matrix_mul(roundKey, key, KMatrix(r, params), tmp1, params);    // roundKey = key * KMatrix(r)
        aux_mpc_sbox(state, tapes, params);
        aux_matrix_mul(state, state, LMatrix(r - 1, params), tmp1, params); // state = state * LMatrix(r-1)
        // NOP: state =  state XOR RConstant(r - 1)
        mpc_xor_masks(state, state, roundKey);
    }

    // Reset the random tape counter so that the online execution uses the
    // same random bits as when computing the aux shares
    tapes->pos = 0;

    freeShares(key);
    freeShares(roundKey);
    freeShares(state);
    freeShares(tmp1);
}

static void commit(uint8_t* digest, uint8_t* seed, uint8_t* aux, uint8_t* salt, size_t t, size_t j, paramset_t* params)
{
    /* Compute C[t][j];  as digest = H(seed||[aux]) aux is optional */
    HashInstance ctx;

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    HashUpdate(&ctx, seed, params->seedSizeBytes);
    if (aux != NULL) {
        size_t tapeLenBytes = params->andSizeBytes;
        HashUpdate(&ctx, aux, tapeLenBytes);
    }
    HashUpdate(&ctx, salt, params->saltSizeBytes);
    HashUpdateIntLE(&ctx, t);
    HashUpdateIntLE(&ctx, j);
    HashFinal(&ctx);
    HashSqueeze(&ctx, digest, params->digestSizeBytes);
}

static void commit_h(uint8_t* digest, commitments_t* C, paramset_t* params)
{
    HashInstance ctx;

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    for (size_t i = 0; i < params->numMPCParties; i++) {
        HashUpdate(&ctx, C->hashes[i], params->seedSizeBytes);
    }
    HashFinal(&ctx);
    HashSqueeze(&ctx, digest, params->digestSizeBytes);
}

// Commit to the views for one parallel rep
static void commit_v(uint8_t* digest, uint8_t* input, msgs_t* msgs, paramset_t* params)
{
    HashInstance ctx;

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    HashUpdate(&ctx, input, params->stateSizeBytes);
    for (size_t i = 0; i < params->numMPCParties; i++) {
        size_t msgs_size = numBytes(msgs->pos);
        HashUpdate(&ctx, msgs->msgs[i], msgs_size);
    }
    HashFinal(&ctx);
    HashSqueeze(&ctx, digest, params->digestSizeBytes);
}

static void reconstructShares(uint32_t* output, shares_t* shares)
{
    for (size_t i = 0; i < shares->numWords; i++) {
        setBitInWordArray(output, i, parity64(shares->shares[i]));
    }
}

static void wordToMsgs(uint64_t w, msgs_t* msgs, paramset_t* params)
{
    for (size_t i = 0; i < params->numMPCParties; i++) {
        uint8_t w_i = getBit((uint8_t*)&w, i);
        setBit(msgs->msgs[i], msgs->pos, w_i);
    }
    msgs->pos++;
}

static uint8_t mpc_AND(uint8_t a, uint8_t b, uint64_t mask_a, uint64_t mask_b, randomTape_t* tapes, msgs_t* msgs, uint64_t* out, paramset_t* params)
{
    uint64_t output_mask = tapesToWord(tapes); // A fresh random mask to hide the result

    *out = output_mask;
    uint64_t and_helper = tapesToWord(tapes);   // The special mask value setup during preprocessing for eahc AND gate
    uint64_t s_shares = (extend(a) & mask_b) ^ (extend(b) & mask_a) ^ and_helper ^ output_mask;

    if (msgs->unopened >= 0) {
        uint8_t unopenedPartyBit = getBit(msgs->msgs[msgs->unopened], msgs->pos);
        setBit((uint8_t*)&s_shares, msgs->unopened, unopenedPartyBit);
    }

    // Broadcast each share of s
    wordToMsgs(s_shares, msgs, params);

    return (uint8_t)(parity64(s_shares) ^ (a & b));
}

static void mpc_sbox(uint32_t* state, shares_t* state_masks, randomTape_t* tapes, msgs_t* msgs, paramset_t* params)
{
    for (size_t i = 0; i < params->numSboxes * 3; i += 3) {
        uint8_t a = getBitFromWordArray(state, i + 2);
        uint64_t mask_a = state_masks->shares[i + 2];

        uint8_t b = getBitFromWordArray(state, i + 1);
        uint64_t mask_b = state_masks->shares[i + 1];

        uint8_t c = getBitFromWordArray(state, i);
        uint64_t mask_c = state_masks->shares[i];

        uint64_t bc_mask, ab_mask, ca_mask; // Fresh output masks used for the AND gate

        uint8_t ab = mpc_AND(a, b, mask_a, mask_b, tapes, msgs, &ab_mask, params);
        uint8_t bc = mpc_AND(b, c, mask_b, mask_c, tapes, msgs, &bc_mask, params);
        uint8_t ca = mpc_AND(c, a, mask_c, mask_a, tapes, msgs, &ca_mask, params);

        setBitInWordArray(state, i + 2, a ^ bc);
        state_masks->shares[i + 2] = mask_a ^ bc_mask;
        setBitInWordArray(state, i + 1, a ^ b ^ ca);
        state_masks->shares[i + 1] = mask_b ^ mask_a ^ ca_mask;
        setBitInWordArray(state, i, a ^ b ^ c ^ ab);
        state_masks->shares[i] = mask_a ^ mask_b ^ mask_c ^ ab_mask;
    }
}

/* For each word in shares; write player i's share to their stream of msgs */
static void broadcast(shares_t* shares, msgs_t* msgs, paramset_t* params)
{
    for (size_t w = 0; w < shares->numWords; w++) {
        wordToMsgs(shares->shares[w], msgs, params);
    }
}

static void mpc_matrix_mul(uint32_t* output, const uint32_t* vec, const uint32_t* matrix, shares_t* mask_shares, paramset_t* params)
{
    uint32_t prod[LOWMC_MAX_STATE_SIZE];
    uint32_t temp[LOWMC_MAX_STATE_SIZE];

    shares_t* tmp_mask = allocateShares(mask_shares->numWords);

    for (size_t i = 0; i < params->stateSizeBits; i++) {
        tmp_mask->shares[i] = 0;
        for (uint32_t j = 0; j < params->stateSizeBits / 8; j++) {
            uint8_t matrix_byte = ((uint8_t*)matrix)[(i * params->stateSizeBits) / 8 + j];
            uint8_t vec_byte = ((uint8_t*)vec)[j];

            ((uint8_t*)prod)[j] = matrix_byte & vec_byte;

            tmp_mask->shares[i] ^= mask_shares->shares[j * 8] & extend((matrix_byte >> 7) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 1] & extend((matrix_byte >> 6) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 2] & extend((matrix_byte >> 5) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 3] & extend((matrix_byte >> 4) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 4] & extend((matrix_byte >> 3) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 5] & extend((matrix_byte >> 2) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 6] & extend((matrix_byte >> 1) & 1);
            tmp_mask->shares[i] ^= mask_shares->shares[j * 8 + 7] & extend(matrix_byte & 1);

        }
        uint8_t output_bit_i = parity(&prod[0], params->stateSizeWords);
        setBit((uint8_t*)temp, i, output_bit_i);
    }

    memcpy(output, &temp, params->stateSizeBytes);
    copyShares(mask_shares, tmp_mask);
    freeShares(tmp_mask);
}

#if 0
/* Alternative, simpler implementation of mpc_matrix_mul, closer to the description in the spec */
static void mpc_matrix_mul_simple(uint32_t* output, const uint32_t* vec, const uint32_t* matrix, shares_t* mask_shares, paramset_t* params)
{
    uint32_t prod[LOWMC_MAX_STATE_SIZE];
    uint32_t temp[LOWMC_MAX_STATE_SIZE];

    shares_t* tmp_mask = allocateShares(mask_shares->numWords);

    for (size_t i = 0; i < params->stateSizeBits; i++) {
        tmp_mask->shares[i] = 0;
        for (uint32_t j = 0; j < params->stateSizeBits; j++) {
            uint8_t matrix_bit = getBit((uint8_t*)matrix, i * params->stateSizeBits + j);
            uint8_t vec_bit = getBit((uint8_t*)vec, j);
            setBit((uint8_t*)prod, j, matrix_bit & vec_bit);
            tmp_mask->shares[i] ^= mask_shares->shares[j] & extend(matrix_bit);
        }
        uint8_t output_bit_i = parity(&prod[0], params->stateSizeWords);
        setBit((uint8_t*)temp, i, output_bit_i);
    }

    memcpy(output, &temp, params->stateSizeBytes);
    copyShares(mask_shares, tmp_mask);
    freeShares(tmp_mask);
}
#endif

static void mpc_xor2(uint32_t* output, shares_t* output_masks, const uint32_t* x,
                     const shares_t* x_masks,  const uint32_t* y, const shares_t* y_masks, paramset_t* params)
{
    xor_array(output, x, y, params->stateSizeWords);
    mpc_xor_masks(output_masks, x_masks, y_masks);
}

#if 0
/* Helper function when debugging MPC function that operate on masked values */
static void print_unmasked(char* label, uint32_t* state, shares_t* mask_shares, paramset_t* params)
{
    uint32_t tmp[LOWMC_MAX_STATE_SIZE];

    reconstructShares(tmp, mask_shares);
    xor_array(tmp, tmp, state, params->stateSizeWords);
    printHex(label, (uint8_t*)tmp, params->stateSizeBytes);
}
#endif



static int contains(uint16_t* list, size_t len, size_t value)
{
    for (size_t i = 0; i < len; i++) {
        if (list[i] == value) {
            return 1;
        }
    }
    return 0;
}

static int indexOf(uint16_t* list, size_t len, size_t value)
{
    for (size_t i = 0; i < len; i++) {
        if (list[i] == value) {
            return i;
        }
    }
    assert(!"indexOf called on list where value is not found. (caller bug)");
    return -1;
}

static void getAuxBits(uint8_t* output, randomTape_t* tapes, paramset_t* params)
{
    size_t firstAuxIndex = params->stateSizeBits + 1;
    size_t last = params->numMPCParties - 1;
    size_t pos = 0;

    memset(output, 0, params->andSizeBytes);
    size_t andSizeBits = 3 * params->numRounds * params->numSboxes;
    for (size_t i = 0; i < andSizeBits * 2; i += 2) {
        uint8_t auxBit = getBit(tapes->tape[last], firstAuxIndex + i);
        setBit(output, pos, auxBit);
        pos++;
    }
}

static void setAuxBits(randomTape_t* tapes, uint8_t* input, paramset_t* params)
{
    size_t firstAuxIndex = params->stateSizeBits + 1;
    size_t last = params->numMPCParties - 1;
    size_t pos = 0;

    for (size_t i = 0; i < params->andSizeBytes * 2 * 8; i += 2) {
        uint8_t auxBit = getBit(input, pos);
        setBit(tapes->tape[last], firstAuxIndex + i, auxBit);
        pos++;
    }
}

static int simulateOnline(uint32_t* maskedKey, shares_t* mask_shares, randomTape_t*
                          tapes, msgs_t* msgs, const uint32_t* plaintext, const uint32_t* pubKey, paramset_t* params)
{
    int ret = 0;
    uint32_t* roundKey = malloc(params->stateSizeBytes);
    uint32_t* state = malloc(params->stateSizeBytes);
    shares_t* key_masks = allocateShares(mask_shares->numWords);    // Make a copy to use when computing each round key

    copyShares(key_masks, mask_shares);

    mpc_matrix_mul(roundKey, maskedKey, KMatrix(0, params), mask_shares, params);       // roundKey = maskedKey * KMatrix[0]
    xor_array(state, roundKey, plaintext, params->stateSizeWords);                      // state = plaintext + roundKey

    shares_t* round_key_masks = allocateShares(mask_shares->numWords);
    for (uint32_t r = 1; r <= params->numRounds; r++) {
        copyShares(round_key_masks, key_masks);
        mpc_matrix_mul(roundKey, maskedKey, KMatrix(r, params), round_key_masks, params);

        mpc_sbox(state, mask_shares, tapes, msgs, params);
        mpc_matrix_mul(state, state, LMatrix(r - 1, params), mask_shares, params);              // state = state * LMatrix (r-1)
        xor_array(state, state, RConstant(r - 1, params), params->stateSizeWords);              // state += RConstant
        mpc_xor2(state, mask_shares, roundKey, round_key_masks, state, mask_shares, params);    // state += roundKey
    }
    freeShares(round_key_masks);

    /* Unmask the output, and check that it's correct */
    if (msgs->unopened >= 0) {
        /* During signature verification we have the shares of the output for
         * the unopened party already in msgs, but not in mask_shares. */
        for (size_t i = 0; i < params->stateSizeBits; i++) {
            uint8_t share = getBit(msgs->msgs[msgs->unopened], msgs->pos + i);
            setBit((uint8_t*)&mask_shares->shares[i],  msgs->unopened, share);
        }

    }
    uint32_t output[LOWMC_MAX_STATE_SIZE];
    reconstructShares(output, mask_shares);
    xor_array(output, output, state, params->stateSizeWords);

    if (memcmp(output, pubKey, params->stateSizeBytes) != 0) {
        printf("%s: output does not match pubKey\n", __func__);
        printHex("pubKey", (uint8_t*)pubKey, params->stateSizeBytes);
        printHex("output", (uint8_t*)output, params->stateSizeBytes);
        ret = -1;
        goto Exit;
    }

    broadcast(mask_shares, msgs, params);

    free(state);
    free(roundKey);
    freeShares(key_masks);

Exit:
    return ret;
}

static size_t bitsToChunks(size_t chunkLenBits, const uint8_t* input, size_t inputLen, uint16_t* chunks)
{
    if (chunkLenBits > inputLen * 8) {
        assert(!"Invalid input to bitsToChunks: not enough input");
        return 0;
    }
    size_t chunkCount = ((inputLen * 8) / chunkLenBits);

    for (size_t i = 0; i < chunkCount; i++) {
        chunks[i] = 0;
        for (size_t j = 0; j < chunkLenBits; j++) {
            chunks[i] += getBit(input, i * chunkLenBits + j) << j;
            assert(chunks[i] < (1 << chunkLenBits));
        }
        chunks[i] = fromLittleEndian(chunks[i]);
    }

    return chunkCount;
}

static size_t appendUnique(uint16_t* list, uint16_t value, size_t position)
{
    if (position == 0) {
        list[position] = value;
        return position + 1;
    }

    for (size_t i = 0; i < position; i++) {
        if (list[i] == value) {
            return position;
        }
    }
    list[position] = value;
    return position + 1;
}

static void HCP(uint16_t* challengeC, uint16_t* challengeP, commitments_t* Ch,
                uint8_t* hCv, uint8_t* salt, const uint32_t* pubKey, const uint32_t* plaintext, const uint8_t* message,
                size_t messageByteLength, paramset_t* params)
{
    HashInstance ctx;
    uint8_t h[MAX_DIGEST_SIZE] = { 0 };

    assert(params->numOpenedRounds < params->numMPCRounds);

#if 0  // Print out inputs when debugging
    printf("\n");
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        printf("%s Ch[%lu]", __func__, t);
        printHex("", Ch->hashes[t], params->digestSizeBytes);

    }
    printHex("hCv", hCv, params->digestSizeBytes);

    printf("%s salt", __func__);
    printHex("", salt, params->saltSizeBytes);
    printf("%s pubKey", __func__);
    printHex("", (uint8_t*)pubKey, params->stateSizeBytes);
    printf("%s plaintext", __func__);
    printHex("", (uint8_t*)plaintext, params->stateSizeBytes);

#endif

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        HashUpdate(&ctx, Ch->hashes[t], params->digestSizeBytes);
    }

    HashUpdate(&ctx, hCv, params->digestSizeBytes);
    HashUpdate(&ctx, salt, params->saltSizeBytes);
    HashUpdate(&ctx, (uint8_t*)pubKey, params->stateSizeBytes);
    HashUpdate(&ctx, (uint8_t*)plaintext, params->stateSizeBytes);
    HashUpdate(&ctx, message, messageByteLength);
    HashFinal(&ctx);
    HashSqueeze(&ctx, h, params->digestSizeBytes);

    // Populate C
    uint32_t bitsPerChunkC = ceil_log2(params->numMPCRounds);
    uint32_t bitsPerChunkP = ceil_log2(params->numMPCParties);
    uint16_t* chunks = calloc(params->digestSizeBytes * 8 / bitsPerChunkP, sizeof(uint16_t));

    size_t countC = 0;
    while (countC < params->numOpenedRounds) {
        size_t numChunks = bitsToChunks(bitsPerChunkC, h, params->digestSizeBytes, chunks);
        for (size_t i = 0; i < numChunks; i++) {
            if (chunks[i] < params->numMPCRounds) {
                countC = appendUnique(challengeC, chunks[i], countC);
            }
            if (countC == params->numOpenedRounds) {
                break;
            }
        }

        HashInit(&ctx, params, HASH_PREFIX_1);
        HashUpdate(&ctx, h, params->digestSizeBytes);
        HashFinal(&ctx);
        HashSqueeze(&ctx, h, params->digestSizeBytes);
    }

    // Note that we always compute h = H(h) after setting C
    size_t countP = 0;

    while (countP < params->numOpenedRounds) {
        size_t numChunks = bitsToChunks(bitsPerChunkP, h, params->digestSizeBytes, chunks);
        for (size_t i = 0; i < numChunks; i++) {
            if (chunks[i] < params->numMPCParties) {
                challengeP[countP] = chunks[i];
                countP++;
            }
            if (countP == params->numOpenedRounds) {
                break;
            }
        }

        HashInit(&ctx, params, HASH_PREFIX_1);
        HashUpdate(&ctx, h, params->digestSizeBytes);
        HashFinal(&ctx);
        HashSqueeze(&ctx, h, params->digestSizeBytes);
    }

#if 0   // Print challenge when debugging
    printf("C = ");
    for (size_t i = 0; i < countC; i++) {
        printf("%u, ", challengeC[i]);
    }
    printf("\n");

    printf("P = ");
    for (size_t i = 0; i < countP; i++) {
        printf("%u, ", challengeP[i]);
    }
    printf("\n");
#endif

    free(chunks);

}

static uint16_t* getMissingLeavesList(uint16_t* challengeC, paramset_t* params)
{
    size_t missingLeavesSize = params->numMPCRounds - params->numOpenedRounds;
    uint16_t* missingLeaves = calloc(missingLeavesSize, sizeof(uint16_t));
    size_t pos = 0;

    for (size_t i = 0; i < params->numMPCRounds; i++) {
        if (!contains(challengeC, params->numOpenedRounds, i)) {
            missingLeaves[pos] = i;
            pos++;
        }
    }

    return missingLeaves;
}

int verify_picnic2(signature2_t* sig, const uint32_t* pubKey, const uint32_t* plaintext, const uint8_t* message, size_t messageByteLength,
                   paramset_t* params)
{
    commitments_t* C = allocateCommitments(params, 0);
    commitments_t Ch = { 0 };
    commitments_t Cv = { 0 };
    msgs_t* msgs = allocateMsgs(params);
    tree_t* treeCv = createTree(params->numMPCRounds, params->digestSizeBytes);
    size_t challengeSizeBytes = params->numOpenedRounds * sizeof(uint16_t);
    uint16_t* challengeC = malloc(challengeSizeBytes);
    uint16_t* challengeP = malloc(challengeSizeBytes);
    tree_t** seeds = calloc(params->numMPCRounds, sizeof(tree_t*));
    randomTape_t* tapes = malloc(params->numMPCRounds * sizeof(randomTape_t));
    tree_t* iSeedsTree = createTree(params->numMPCRounds, params->seedSizeBytes);
    int ret = reconstructSeeds(iSeedsTree, sig->challengeC, params->numOpenedRounds, sig->iSeedInfo, sig->iSeedInfoLen, sig->salt, 0, params);

    if (ret != 0) {
        ret = -1;
        goto Exit;
    }

    /* Populate seeds with values from the signature */
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        if (!contains(sig->challengeC, params->numOpenedRounds, t)) {
            /* Expand iSeed[t] to seeds for each parties, using a seed tree */
            seeds[t] = generateSeeds(params->numMPCParties, getLeaf(iSeedsTree, t), sig->salt, t, params);
        }
        else {
            /* We don't have the initial seed for the round, but instead a seed
             * for each unopened party */
            seeds[t] = createTree(params->numMPCParties, params->seedSizeBytes);
            size_t P_index = indexOf(sig->challengeC, params->numOpenedRounds, t);
            uint16_t hideList[1];
            hideList[0] = sig->challengeP[P_index];
            ret = reconstructSeeds(seeds[t], hideList, 1,
                                   sig->proofs[t].seedInfo, sig->proofs[t].seedInfoLen,
                                   sig->salt, t, params);
            if (ret != 0) {
                printf("Failed to reconstruct seeds for round %lu\n", t);
                ret = -1;
                goto Exit;
            }
        }
    }

    /* Commit */
    size_t last = params->numMPCParties - 1;
    uint8_t auxBits[MAX_AUX_BYTES];
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        /* Compute random tapes for all parties.  One party for each repitition
         * challengeC will have a bogus seed; but we won't use that party's
         * random tape. */
        createRandomTapes(&tapes[t], getLeaves(seeds[t]), sig->salt, t, params);

        if (!contains(sig->challengeC, params->numOpenedRounds, t)) {
            /* We're given iSeed, have expanded the seeds, compute aux from scratch so we can comnpte Com[t] */
            computeAuxTape(&tapes[t], params);
            for (size_t j = 0; j < last; j++) {
                commit(C[t].hashes[j], getLeaf(seeds[t], j), NULL, sig->salt, t, j, params);
            }
            getAuxBits(auxBits, &tapes[t], params);
            commit(C[t].hashes[last], getLeaf(seeds[t], last), auxBits, sig->salt, t, last, params);
        }
        else {
            /* We're given all seeds and aux bits, execpt for the unopened 
             * party, we get their commitment */
            size_t unopened = sig->challengeP[indexOf(sig->challengeC, params->numOpenedRounds, t)];
            for (size_t j = 0; j < last; j++) {
                if (j != unopened) {
                    commit(C[t].hashes[j], getLeaf(seeds[t], j), NULL, sig->salt, t, j, params);
                }
            }
            if (last != unopened) {
                commit(C[t].hashes[last], getLeaf(seeds[t], last), sig->proofs[t].aux, sig->salt, t, last, params);
            }

            memcpy(C[t].hashes[unopened], sig->proofs[t].C, params->digestSizeBytes);
        }

    }


    /* Commit to the commitments */
    allocateCommitments2(&Ch, params, params->numMPCRounds);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        commit_h(Ch.hashes[t], &C[t], params);
    }

    /* Commit to the views */
    allocateCommitments2(&Cv, params, params->numMPCRounds);
    shares_t* mask_shares = allocateShares(params->stateSizeBits);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        if (contains(sig->challengeC, params->numOpenedRounds, t)) {
            /* 2. When t is in C, we have everything we need to re-compute the view, as an honest signer would.
             * We simulate the MPC with one fewer party; the unopned party's values are all set to zero. */
            size_t unopened = sig->challengeP[indexOf(sig->challengeC, params->numOpenedRounds, t)];
            size_t tapeLengthBytes = 2 * params->andSizeBytes + params->stateSizeBytes;
            if(unopened != last) {
                setAuxBits(&tapes[t], sig->proofs[t].aux, params);  // sig->proofs[t].aux is only set when P_t != N
            }
            memset(tapes[t].tape[unopened], 0, tapeLengthBytes);
            memcpy(msgs[t].msgs[unopened], sig->proofs[t].msgs, params->andSizeBytes + params->stateSizeBytes );
            msgs[t].unopened = unopened;

            tapesToWords(mask_shares, &tapes[t]);
            int ret = simulateOnline((uint32_t*)sig->proofs[t].input, mask_shares, &tapes[t], &msgs[t], plaintext, pubKey, params);
            if (ret != 0) {
                printf("MPC simulation failed for round %lu, signature invalid\n", t);
                ret = -1;
                freeShares(mask_shares);
                goto Exit;
            }
            commit_v(Cv.hashes[t], sig->proofs[t].input, &msgs[t], params);
        }
        else {
            Cv.hashes[t] = NULL;
        }
    }
    freeShares(mask_shares);

    size_t missingLeavesSize = params->numMPCRounds - params->numOpenedRounds;
    uint16_t* missingLeaves = getMissingLeavesList(sig->challengeC, params);
    ret = addMerkleNodes(treeCv, missingLeaves, missingLeavesSize, sig->cvInfo, sig->cvInfoLen);
    free(missingLeaves);
    if (ret != 0) {
        ret = -1;
        goto Exit;
    }

    ret = verifyMerkleTree(treeCv, Cv.hashes, sig->salt, params);
    if (ret != 0) {
        ret = -1;
        goto Exit;
    }

    /* Compute the challenge; two lists of integers */
    HCP(challengeC, challengeP, &Ch, treeCv->nodes[0], sig->salt, pubKey, plaintext, message, messageByteLength, params);

    /* Compare to challenge from signature */
    if ( memcmp(sig->challengeC, challengeC, challengeSizeBytes) != 0 ||
         memcmp(sig->challengeP, challengeP, challengeSizeBytes) != 0 ) {
        printf("Challenge does not match, signature invalid\n");
        ret = -1;
        goto Exit;
    }

    ret = EXIT_SUCCESS;

Exit:

    free(challengeC);
    free(challengeP);
    freeCommitments(C);
    freeCommitments2(&Cv);
    freeCommitments2(&Ch);
    freeMsgs(msgs);
    freeTree(treeCv);
    freeTree(iSeedsTree);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        freeRandomTape(&tapes[t]);
        freeTree(seeds[t]);
    }
    free(seeds);
    free(tapes);

    return ret;
}

static void computeSaltAndRootSeed(uint8_t* saltAndRoot, size_t saltAndRootLength, uint32_t* privateKey, uint32_t* pubKey,
                                   uint32_t* plaintext, const uint8_t* message, size_t messageByteLength, paramset_t* params)
{
    HashInstance ctx;

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    HashUpdate(&ctx, (uint8_t*)privateKey, params->stateSizeBytes);
    HashUpdate(&ctx, message, messageByteLength);
    HashUpdate(&ctx, (uint8_t*)pubKey, params->stateSizeBytes);
    HashUpdate(&ctx, (uint8_t*)plaintext, params->stateSizeBytes);
    HashUpdateIntLE(&ctx, params->stateSizeBits);
    HashFinal(&ctx);
    HashSqueeze(&ctx, saltAndRoot, saltAndRootLength);
}

int sign_picnic2(uint32_t* privateKey, uint32_t* pubKey, uint32_t* plaintext, const uint8_t* message,
                 size_t messageByteLength, signature2_t* sig, paramset_t* params)
{
    int ret = 0;
    uint8_t* saltAndRoot = malloc(params->saltSizeBytes + params->seedSizeBytes);

    computeSaltAndRootSeed(saltAndRoot, params->saltSizeBytes + params->seedSizeBytes, privateKey, pubKey, plaintext, message, messageByteLength, params);
    memcpy(sig->salt, saltAndRoot, params->saltSizeBytes);
    tree_t* iSeedsTree = generateSeeds(params->numMPCRounds, saltAndRoot + params->saltSizeBytes, sig->salt, 0, params);
    uint8_t** iSeeds = getLeaves(iSeedsTree);
    free(saltAndRoot);

    randomTape_t* tapes = malloc(params->numMPCRounds * sizeof(randomTape_t));
    tree_t** seeds = malloc(params->numMPCRounds * sizeof(tree_t*));
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        seeds[t] = generateSeeds(params->numMPCParties, iSeeds[t], sig->salt, t, params);
        createRandomTapes(&tapes[t], getLeaves(seeds[t]), sig->salt, t, params);
    }

    /* Preprocessing; compute aux tape for the N-th player, for each parallel rep */
    uint8_t auxBits[MAX_AUX_BYTES];
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        computeAuxTape(&tapes[t], params);
    }

    /* Commit to seeds and aux bits */
    commitments_t* C = allocateCommitments(params, 0);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        for (size_t j = 0; j < params->numMPCParties - 1; j++) {
            commit(C[t].hashes[j], getLeaf(seeds[t], j), NULL, sig->salt, t, j, params);
        }
        size_t last = params->numMPCParties - 1;
        getAuxBits(auxBits, &tapes[t], params);
        commit(C[t].hashes[last], getLeaf(seeds[t], last), auxBits, sig->salt, t, last, params);
    }

    /* Simulate the online phase of the MPC */
    inputs_t inputs = allocateInputs(params);
    msgs_t* msgs = allocateMsgs(params);
    shares_t* mask_shares = allocateShares(params->stateSizeBits);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        uint32_t* maskedKey = (uint32_t*)inputs[t];

        tapesToWords(mask_shares, &tapes[t]);
        reconstructShares(maskedKey, mask_shares);                                      // maskedKey = masks
        xor_array(maskedKey, maskedKey, privateKey, params->stateSizeWords);            // maskedKey += privateKey

        int rv = simulateOnline(maskedKey, mask_shares, &tapes[t], &msgs[t], plaintext, pubKey, params);
        if (rv != 0) {
            printf("MPC simulation failed, aborting signature\n");
            ret = -1;
        }
    }
    freeShares(mask_shares);

    /* Commit to the commitments and views */
    commitments_t Ch;
    allocateCommitments2(&Ch, params, params->numMPCRounds);
    commitments_t Cv;
    allocateCommitments2(&Cv, params, params->numMPCRounds);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        commit_h(Ch.hashes[t], &C[t], params);
        commit_v(Cv.hashes[t], inputs[t], &msgs[t], params);
    }

    /* Create a Merkle tree with Cv as the leaves */
    tree_t* treeCv = createTree(params->numMPCRounds, params->digestSizeBytes);
    buildMerkleTree(treeCv, Cv.hashes, sig->salt, params);

    /* Compute the challenge; two lists of integers */
    uint16_t* challengeC = sig->challengeC;
    uint16_t* challengeP = sig->challengeP;
    HCP(challengeC, challengeP, &Ch, treeCv->nodes[0], sig->salt, pubKey, plaintext, message, messageByteLength, params);

    /* Send information required for checking commitments with Merkle tree.
     * The commitments the verifier will be missing are those not in challengeC. */
    size_t missingLeavesSize = params->numMPCRounds - params->numOpenedRounds;
    uint16_t* missingLeaves = getMissingLeavesList(challengeC, params);
    size_t cvInfoLen = 0;
    uint8_t* cvInfo = openMerkleTree(treeCv, missingLeaves, missingLeavesSize, &cvInfoLen);
    sig->cvInfo = cvInfo;
    sig->cvInfoLen = cvInfoLen;
    free(missingLeaves);

    /* Reveal iSeeds for unopned rounds, those in {0..T-1} \ ChallengeC. */
    sig->iSeedInfo = malloc(params->numMPCRounds * params->seedSizeBytes);
    sig->iSeedInfoLen = revealSeeds(iSeedsTree, challengeC, params->numOpenedRounds,
                                    sig->iSeedInfo, params->numMPCRounds * params->seedSizeBytes, params);
    sig->iSeedInfo = realloc(sig->iSeedInfo, sig->iSeedInfoLen);

    /* Assemble the proof */
    proof2_t* proofs = sig->proofs;
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        if (contains(challengeC, params->numOpenedRounds, t)) {
            allocateProof2(&proofs[t], params);
            size_t P_index = indexOf(challengeC, params->numOpenedRounds, t);
            proofs[t].unOpenedIndex = challengeP[P_index];

            uint16_t hideList[1];
            hideList[0] = challengeP[P_index];
            proofs[t].seedInfo = malloc(params->numMPCParties * params->seedSizeBytes);
            proofs[t].seedInfoLen = revealSeeds(seeds[t], hideList, 1, proofs[t].seedInfo, params->numMPCParties * params->seedSizeBytes, params);
            proofs[t].seedInfo = realloc(proofs[t].seedInfo, proofs[t].seedInfoLen);

            size_t last = params->numMPCParties - 1;
            if (challengeP[P_index] != last) {
                getAuxBits(proofs[t].aux, &tapes[t], params);
            }

            memcpy(proofs[t].input, inputs[t], params->stateSizeBytes);
            memcpy(proofs[t].msgs, msgs[t].msgs[challengeP[P_index]], params->andSizeBytes + params->stateSizeBytes );
            memcpy(proofs[t].C, C[t].hashes[proofs[t].unOpenedIndex], params->digestSizeBytes);
        }
    }

    sig->proofs = proofs;

#if 0
    printf("\n-----------------\n\nSelf-Test, trying to verify signature:\n");
    int ret = verify_picnic2(sig, pubKey, plaintext, message, messageByteLength, params);
    if (ret != 0) {
        printf("Verification failed; signature invalid\n");
        ret = -1;
    }
    else {
        printf("Verification succeeded\n\n");
    }
    printf("-----------------\n\nSelf-Test complete\n");

#endif

    for (size_t t = 0; t < params->numMPCRounds; t++) {
        freeRandomTape(&tapes[t]);
        freeTree(seeds[t]);
    }
    free(tapes);
    free(seeds);
    freeTree(iSeedsTree);
    freeTree(treeCv);

    freeCommitments(C);
    freeCommitments2(&Ch);
    freeCommitments2(&Cv);
    freeInputs(inputs);
    freeMsgs(msgs);

    return ret;

}


static int inRange(uint16_t* list, size_t len, size_t low, size_t high)
{
    for (size_t i = 0; i < len; i++) {
        if (list[i] > high || list[i] < low) {
            return 0;
        }
    }
    return 1;
}

static int unique(uint16_t* list, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len; j++) {
            if (j != i && list[i] == list[j]) {
                return 0;
            }
        }
    }
    return 1;
}

static int arePaddingBitsZero(uint8_t* data, size_t byteLength, size_t bitLength)
{
    for (size_t i = bitLength; i < byteLength * 8; i++) {
        uint8_t bit_i = getBit(data, i);
        if (bit_i != 0) {
            return 0;
        }
    }
    return 1;
}

int deserializeSignature2(signature2_t* sig, const uint8_t* sigBytes, size_t sigBytesLen, paramset_t* params)
{
    /* Read the challenge and salt */
    size_t bytesRequired = 4 * params->numOpenedRounds + params->saltSizeBytes;

    if (sigBytesLen < bytesRequired) {
        return EXIT_FAILURE;
    }

    memcpy(sig->challengeC, sigBytes, 2 * params->numOpenedRounds);
    sigBytes += 2 * params->numOpenedRounds;
    memcpy(sig->challengeP, sigBytes, 2 * params->numOpenedRounds);
    sigBytes += 2 * params->numOpenedRounds;
    memcpy(sig->salt, sigBytes, params->saltSizeBytes);
    sigBytes += params->saltSizeBytes;

    for (size_t i = 0; i < params->numOpenedRounds; i++) {
        sig->challengeC[i] = fromLittleEndian(sig->challengeC[i]);
        sig->challengeP[i] = fromLittleEndian(sig->challengeP[i]);
    }

    if (!inRange(sig->challengeC, params->numOpenedRounds, 0, params->numMPCRounds - 1)) {
        return EXIT_FAILURE;
    }
    if (!unique(sig->challengeC, params->numOpenedRounds)) {
        return EXIT_FAILURE;
    }
    if (!inRange(sig->challengeP, params->numOpenedRounds, 0, params->numMPCParties - 1)) {
        return EXIT_FAILURE;
    }

    /* Add size of iSeeds tree data */
    sig->iSeedInfoLen = revealSeedsSize(params->numMPCRounds, sig->challengeC, params->numOpenedRounds, params);
    bytesRequired += sig->iSeedInfoLen;

    /* Add the size of the Cv Merkle tree data */
    size_t missingLeavesSize = params->numMPCRounds - params->numOpenedRounds;
    uint16_t* missingLeaves = getMissingLeavesList(sig->challengeC, params);
    sig->cvInfoLen = openMerkleTreeSize(params->numMPCRounds, missingLeaves, missingLeavesSize, params);
    bytesRequired += sig->cvInfoLen;
    free(missingLeaves);

    /* Compute the number of bytes required for the proofs */
    uint16_t hideList[1] = { 0 };
    size_t seedInfoLen = revealSeedsSize(params->numMPCParties, hideList, 1, params);
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        if (contains(sig->challengeC, params->numOpenedRounds, t)) {
            size_t P_t = sig->challengeP[indexOf(sig->challengeC, params->numOpenedRounds, t)];
            if (P_t != (params->numMPCParties - 1)) {
                bytesRequired += params->andSizeBytes;
            }
            bytesRequired += params->digestSizeBytes;
            bytesRequired += params->stateSizeBytes;
            bytesRequired += params->stateSizeBytes + params->andSizeBytes;
            bytesRequired += seedInfoLen;
        }
    }

    /* Fail if the signature does not have the exact number of bytes we expect */
    if (sigBytesLen != bytesRequired) {
        printf("%s: sigBytesLen = %lu, expected bytesRequired = %lu\n", __func__, sigBytesLen, bytesRequired);
        return EXIT_FAILURE;
    }

    sig->iSeedInfo = malloc(sig->iSeedInfoLen);
    memcpy(sig->iSeedInfo, sigBytes, sig->iSeedInfoLen);
    sigBytes += sig->iSeedInfoLen;

    sig->cvInfo = malloc(sig->cvInfoLen);
    memcpy(sig->cvInfo, sigBytes, sig->cvInfoLen);
    sigBytes += sig->cvInfoLen;

    /* Read the proofs */
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        if (contains(sig->challengeC, params->numOpenedRounds, t)) {
            allocateProof2(&sig->proofs[t], params);
            sig->proofs[t].seedInfoLen = seedInfoLen;
            sig->proofs[t].seedInfo = malloc(sig->proofs[t].seedInfoLen);
            memcpy(sig->proofs[t].seedInfo, sigBytes, sig->proofs[t].seedInfoLen);
            sigBytes += sig->proofs[t].seedInfoLen;

            size_t P_t = sig->challengeP[indexOf(sig->challengeC, params->numOpenedRounds, t)];
            if (P_t != (params->numMPCParties - 1) ) {
                memcpy(sig->proofs[t].aux, sigBytes, params->andSizeBytes);
                sigBytes += params->andSizeBytes;
                if (!arePaddingBitsZero(sig->proofs[t].aux, params->andSizeBytes, 3 * params->numRounds * params->numSboxes)) {
                    printf("%s: failed while deserializing aux bits\n", __func__);
                    return -1;
                }
            }

            memcpy(sig->proofs[t].input, sigBytes, params->seedSizeBytes);
            sigBytes += params->stateSizeBytes;

            size_t msgsByteLength = params->stateSizeBytes + params->andSizeBytes;
            memcpy(sig->proofs[t].msgs, sigBytes, msgsByteLength);
            sigBytes += msgsByteLength;
            size_t msgsBitLength = params->stateSizeBits + 3 * params->numRounds * params->numSboxes;
            if (!arePaddingBitsZero(sig->proofs[t].msgs, msgsByteLength, msgsBitLength)) {
                printf("%s: failed while deserializing msgs bits\n", __func__);
                return -1;
            }

            memcpy(sig->proofs[t].C, sigBytes, params->digestSizeBytes);
            sigBytes += params->digestSizeBytes;
        }
    }

    return EXIT_SUCCESS;
}

int serializeSignature2(const signature2_t* sig, uint8_t* sigBytes, size_t sigBytesLen, paramset_t* params)
{
    uint8_t* sigBytesBase = sigBytes;

    /* Compute the number of bytes required for the signature */
    size_t bytesRequired = 4 * params->numOpenedRounds + params->saltSizeBytes; /* challenge and salt */

    bytesRequired += sig->iSeedInfoLen;                                         /* Encode only iSeedInfo, the length will be recomputed by deserialize */
    bytesRequired += sig->cvInfoLen;

    for (size_t t = 0; t < params->numMPCRounds; t++) {   /* proofs */
        if (contains(sig->challengeC, params->numOpenedRounds, t)) {
            size_t P_t = sig->challengeP[indexOf(sig->challengeC, params->numOpenedRounds, t)];
            bytesRequired += sig->proofs[t].seedInfoLen;
            if (P_t != (params->numMPCParties - 1)) {
                bytesRequired += params->andSizeBytes;
            }
            bytesRequired += params->digestSizeBytes;
            bytesRequired += params->stateSizeBytes;
            bytesRequired += params->stateSizeBytes + params->andSizeBytes;
        }
    }

    if (sigBytesLen < bytesRequired) {
        return -1;
    }

    memcpy(sigBytes, sig->challengeC, 2 * params->numOpenedRounds);
    uint16_t* challengeC = (uint16_t*)sigBytes;
    sigBytes += 2 * params->numOpenedRounds;
    memcpy(sigBytes, sig->challengeP, 2 * params->numOpenedRounds);
    uint16_t* challengeP = (uint16_t*)sigBytes;
    sigBytes += 2 * params->numOpenedRounds;
    memcpy(sigBytes, sig->salt, params->saltSizeBytes);
    sigBytes += params->saltSizeBytes;

    for (size_t i = 0; i < params->numOpenedRounds; i++) {
        challengeC[i] = fromLittleEndian(sig->challengeC[i]);
        challengeP[i] = fromLittleEndian(sig->challengeP[i]);
    }

    memcpy(sigBytes, sig->iSeedInfo, sig->iSeedInfoLen);
    sigBytes += sig->iSeedInfoLen;
    memcpy(sigBytes, sig->cvInfo, sig->cvInfoLen);
    sigBytes += sig->cvInfoLen;

    /* Write the proofs */
    for (size_t t = 0; t < params->numMPCRounds; t++) {
        if (contains(sig->challengeC, params->numOpenedRounds, t)) {
            memcpy(sigBytes, sig->proofs[t].seedInfo,  sig->proofs[t].seedInfoLen);
            sigBytes += sig->proofs[t].seedInfoLen;

            size_t P_t = sig->challengeP[indexOf(sig->challengeC, params->numOpenedRounds, t)];

            if (P_t != (params->numMPCParties - 1) ) {
                memcpy(sigBytes, sig->proofs[t].aux, params->andSizeBytes);
                sigBytes += params->andSizeBytes;
            }

            memcpy(sigBytes, sig->proofs[t].input, params->seedSizeBytes);
            sigBytes += params->stateSizeBytes;

            memcpy(sigBytes, sig->proofs[t].msgs, params->stateSizeBytes + params->andSizeBytes);
            sigBytes += params->stateSizeBytes + params->andSizeBytes;

            memcpy(sigBytes, sig->proofs[t].C, params->digestSizeBytes);
            sigBytes += params->digestSizeBytes;
        }
    }

    return (int)(sigBytes - sigBytesBase);
}

