/**
Custom RNG generator. By default, uses the hardware based /dev/urandom.
By calling randombytes_set, can register an external method for producing random bytes.
*/

#include "cqcrandom.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/// A function pointer that will store the location of the randombytes method.
/// unsigned char* : The buffer that will hold the generated data.
/// unsigned int : the size of the buffer ( the randombytes method should fill this buffer exactly).
int (*_rb_instance)(unsigned char*, unsigned int) = 0;


void randombytes_set(int (*instance)(unsigned char*, unsigned int) ) {
    _rb_instance = instance;
}

static int lock = -1;

static __inline void delay(unsigned int count)
{
	while (count--) {}
}

/// Generate random bytes and output the result to random_array
/// @param random_array The buffer that will hold the generated data.
/// @param nbytes The size of the buffer.
int randombytes(unsigned char* random_array, unsigned int nbytes)
{
    
    if (_rb_instance != 0 ) {
        return _rb_instance(random_array, nbytes);
    }
    
    // Generation of "nbytes" of random values
	int r, n = nbytes, count = 0;
    
    if (lock == -1) {
	    do {
		    lock = open("/dev/urandom", O_RDONLY);
		    if (lock == -1) {
			    delay(0xFFFFF);
		    }
	    } while (lock == -1);
    }

	while (n > 0) {
		do {
			r = read(lock, random_array+count, n);
			if (r == -1) {
				delay(0xFFFF);
			}
		} while (r == -1);
		count += r;
		n -= r;
	}
    
	return 0;
}


