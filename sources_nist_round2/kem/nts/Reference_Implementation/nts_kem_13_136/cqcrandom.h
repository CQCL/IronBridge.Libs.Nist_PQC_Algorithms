#ifndef __CQCRANDOM_H__
#define __CQCRANDOM_H__

// Generate random bytes and output the result to random_array
int randombytes(unsigned char* random_array, unsigned int nbytes);

// Register an external random bytes producer.
void randombytes_set(int (*instance)(unsigned char*, unsigned int) );

#endif
