CC=/usr/bin/gcc
LDFLAGS=-lcrypto

CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c
SOURCES= cbd.c indcpa.c kem.c ntt.c poly.c polyvec.c PQCgenKAT_kem.c reduce.c rng.c verify.c sha256.c sha512.c aes256ctr.c
LIB_SOURCES_CQC= cbd.c indcpa.c kem.c ntt.c poly.c polyvec.c reduce.c $(CQCRANDOM_SRC) verify.c sha256.c sha512.c aes256ctr.c fips202.c symmetric-fips202.c
HEADERS= api.h cbd.h indcpa.h ntt.h params.h poly.h polyvec.h reduce.h rng.h verify.h symmetric.h sha2.h aes256ctr.h

PQCgenKAT_kem: $(HEADERS) $(SOURCES)
	$(CC) -DKYBER_90S -o $@ $(SOURCES) $(LDFLAGS)

libkyber-51290s_NR2_CQCRNG.so: $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC)  -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: libkyber-51290s_NR2_CQCRNG.so 

.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem
	-rm -f libkyber-51290s_NR2_CQCRNG.so

