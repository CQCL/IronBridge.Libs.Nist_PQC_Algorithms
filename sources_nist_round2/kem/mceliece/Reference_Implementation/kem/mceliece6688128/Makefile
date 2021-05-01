kat_kem.rsp: kat
	./run

kat: Makefile nist/kat_kem.c nist/rng.c nist/rng.h randombytes.h benes.c bm.c controlbits.c decrypt.c encrypt.c gf.c operations.c pk_gen.c root.c sk_gen.c synd.c transpose.c util.c
	./build


DIR := ${CURDIR}
CC=/usr/bin/gcc -fPIC
CFLAGS= -fPIC -O3 -march=native -mtune=native -Wall  -I/$(DIR)
LDFLAGS= -lcrypto -ldl -L. -lkeccak
LIBS = -L${CURDIR}/libkeccak.a
CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c

LIB_SOURCES_CQC= $(CQCRANDOM_SRC) benes.c bm.c controlbits.c decrypt.c encrypt.c gf.c operations.c pk_gen.c root.c sk_gen.c synd.c transpose.c util.c crypto_stream_aes256ctr.c
HEADERS= 

libmceliece-6688128_NR2_CQCRNG.so: $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC) $(CFLAGS) -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: libmceliece-6688128_NR2_CQCRNG.so

clean:
	-rm -f libmceliece-6688128_NR2_CQCRNG.so