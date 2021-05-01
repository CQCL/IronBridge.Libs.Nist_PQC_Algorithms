CC=/usr/bin/gcc
LDFLAGS=-lcrypto

CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c
SOURCES= cbd.c fips202.c indcpa.c kem.c ntt.c poly.c polyvec.c PQCgenKAT_kem.c reduce.c rng.c verify.c symmetric-fips202.c
LIB_SOURCES_CQC= cbd.c fips202.c indcpa.c kem.c ntt.c poly.c polyvec.c reduce.c $(CQCRANDOM_SRC) verify.c symmetric-fips202.c
HEADERS= api.h cbd.h fips202.h indcpa.h ntt.h params.h poly.h polyvec.h reduce.h rng.h verify.h symmetric.h

PQCgenKAT_kem: $(HEADERS) $(SOURCES)
	$(CC) -o $@ $(SOURCES) $(LDFLAGS)

libkyber-768_NR2_CQCRNG.so: $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC)  -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: libkyber-768_NR2_CQCRNG.so 


.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem
	-rm -f libkyber-768_NR2_CQCRNG.so

