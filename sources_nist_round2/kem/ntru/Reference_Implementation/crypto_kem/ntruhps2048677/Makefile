CC=/usr/bin/gcc
LDFLAGS=-lcrypto

CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c
SOURCES = crypto_sort.c fips202.c kem.c owcpa.c pack3.c packq.c poly.c sample.c verify.c rng.c PQCgenKAT_kem.c
LIB_SOURCES_CQC = crypto_sort.c fips202.c kem.c owcpa.c pack3.c packq.c poly.c sample.c verify.c $(CQCRANDOM_SRC)
HEADERS = api.h crypto_sort.h fips202.h kem.h poly.h owcpa.h params.h sample.h verify.h

PQCgenKAT_kem: $(HEADERS) $(SOURCES)
	$(CC) -o $@ $(SOURCES) $(LDFLAGS)

libntru-hps2048677_NR2_CQCRNG.so: $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC)  -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: libntru-hps2048677_NR2_CQCRNG.so

.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem
	-rm -f libntru-hps2048677_NR2_CQCRNG.so

