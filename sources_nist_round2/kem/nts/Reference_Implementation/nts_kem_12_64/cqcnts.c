//
//  cqcnts.c
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "nts_kem_errors.h"


#ifndef PARAM_RND_SIZE
#define PARAM_RND_SIZE      16
#define PARAM_RND_BIT_SIZE  128
#endif


int randombytes(unsigned char* random_array, unsigned int nbytes);
uint16_t random_uint16_bounded(uint16_t bound);
uint8_t randombit();

 
uint8_t randombit()
{
    static int32_t bits_consumed = PARAM_RND_BIT_SIZE;
    static uint8_t rnd_buffer[PARAM_RND_SIZE];
    uint8_t b = 0;
    
    /**
     * Have we depleted our random source?
     **/
    if (bits_consumed >= PARAM_RND_BIT_SIZE) {
        /**
         * If so, generate PARAM_RND_SIZE bytes
         * of random data as our random source
         **/
        randombytes(rnd_buffer, sizeof(rnd_buffer));
        bits_consumed = 0;
    }
    
    b = (rnd_buffer[bits_consumed >> 3] & (1 << (bits_consumed & 7))) >> (bits_consumed & 7);
    bits_consumed++;
    
    return b;
}

uint16_t random_uint16_bounded(uint16_t bound)
{
    uint16_t d, u, x;
    
    /* Knuth-Yao DDG */
    d = 0; u = 1; x = 0;
    do {
        while (u < bound) {
            u = 2*u;
            x = 2*x + randombit();
        }
        d = u - bound;
        u = d;
    } while (x < d);
    
    return x - d;
}
   
