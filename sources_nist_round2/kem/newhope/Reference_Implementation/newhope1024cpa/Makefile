CC = /usr/bin/gcc
LDFLAGS = -lcrypto

CQCRANDOM_SRC = ../../../../cqcrandom/cqcrandom.c
SOURCES = kem.c cpapke.c fips202.c ntt.c poly.c precomp.c reduce.c rng.c verify.c PQCgenKAT_kem.c
LIB_SOURCES_CQC = kem.c cpapke.c fips202.c ntt.c poly.c precomp.c reduce.c $(CQCRANDOM_SRC) verify.c
HEADERS = api.h cpapke.h fips202.h ntt.h params.h poly.h reduce.h verify.h

PQCgenKAT_kem: $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) -o $@ $(LDFLAGS)

libnewhope-1024cpa_NR2_CQCRNG.so: $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC)  -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: libnewhope-1024cpa_NR2_CQCRNG.so

.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem
	-rm -f libnewhope-1024cpa_NR2_CQCRNG.so
