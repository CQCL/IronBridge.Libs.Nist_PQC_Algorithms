kat_kem.rsp: kat
	./run

kat: Makefile nist/kat_kem.c nist/rng.c nist/rng.h randombytes.h benes.c bm.c controlbits.c decrypt.c encrypt.c gf.c operations.c pk_gen.c root.c sk_gen.c synd.c transpose.c util.c
	./build

LIB_TARGET_CQC = libmceliece-348864_NR3_CQCRNG.so
CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c

DIR := ${CURDIR}
CC=/usr/bin/gcc -fPIC
CFLAGS= -fPIC -O3 -march=native -mtune=native -Wall  -I/$(DIR)
LDFLAGS= -lcrypto -ldl -L. -lkeccak
LIBS = -L${CURDIR}/libkeccak.a

LIB_SOURCES_CQC= $(CQCRANDOM_SRC) benes.c bm.c controlbits.c decrypt.c encrypt.c gf.c operations.c pk_gen.c root.c sk_gen.c synd.c transpose.c util.c crypto_stream_aes256ctr.c
HEADERS= 

$(LIB_TARGET_CQC): $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC) $(CFLAGS) -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: $(LIB_TARGET_CQC)

clean:
	-rm -f $(LIB_TARGET_CQC)
