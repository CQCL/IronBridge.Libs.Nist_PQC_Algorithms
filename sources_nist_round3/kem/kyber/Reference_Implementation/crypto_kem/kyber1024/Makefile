CC=/usr/bin/gcc
CFLAGS += -O3 -march=native -fomit-frame-pointer
LDFLAGS=-lcrypto

LIB_TARGET_CQC = libkyber-1024_NR3_CQCRNG.so
CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c

SOURCES= cbd.c fips202.c indcpa.c kem.c ntt.c poly.c polyvec.c PQCgenKAT_kem.c reduce.c rng.c verify.c symmetric-shake.c
LIB_SOURCES_CQC= cbd.c fips202.c indcpa.c kem.c ntt.c poly.c polyvec.c reduce.c $(CQCRANDOM_SRC) verify.c symmetric-shake.c
HEADERS= api.h cbd.h fips202.h indcpa.h ntt.h params.h poly.h polyvec.h reduce.h rng.h verify.h symmetric.h

PQCgenKAT_kem: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

$(LIB_TARGET_CQC): $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC) $(CFLAGS) -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: $(LIB_TARGET_CQC) 

.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem
	-rm -f $(LIB_TARGET_CQC)

