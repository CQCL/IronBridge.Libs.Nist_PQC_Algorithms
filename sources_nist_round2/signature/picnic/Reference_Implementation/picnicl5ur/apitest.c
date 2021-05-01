/*! @file apitest.c
 *  @brief Simple test of the SUPERCOP API
 *
 *  This file is part of the reference implementation of the Picnic signature scheme.
 *  See the accompanying documentation for complete details.
 *
 *  The code is provided under the MIT license, see LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */


#include "api.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

int main()
{
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char m[50] = { 0 };
    unsigned char mprime[50] = { 0 };
    unsigned char sm[sizeof(m) + CRYPTO_BYTES];

    int ret = crypto_sign_keypair(pk, sk);

    if (ret != 0) {
        printf("Failed to generate key pair\n");
        return -1;
    }

    long long unsigned int smlen = sizeof(sm);
    ret = crypto_sign(sm, &smlen, m, sizeof(m), sk);
    if (ret != 0) {
        printf("Failed to sign\n");
        return -1;
    }

    long long unsigned int mlen = sizeof(mprime);
    ret = crypto_sign_open(mprime, &mlen, sm, smlen, pk);
    if (ret != 0) {
        printf("Failed to verify (ret = %d)\n", ret);
        return -1;
    }

    if (mlen != sizeof(m)) {
        printf("length of message after verify incorrect, got %llu, expected%" PRIuPTR "\n", mlen, sizeof(m));
        return -1;
    }
    if (memcmp(m, mprime, mlen) != 0) {
        printf("message mismatch after verification\n");
        return -1;
    }

    printf("Sign/Verify test passed\n");

    return 0;
}
