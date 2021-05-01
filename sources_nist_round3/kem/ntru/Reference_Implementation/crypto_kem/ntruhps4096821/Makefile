
CC=/usr/bin/cc
LDFLAGS=-lcrypto
CFLAGS = -DCRYPTO_NAMESPACE\(s\)=ntru_\#\#s

LIB_TARGET_RNG = libntru-hps4096821_NR3_STDRNG.so
LIB_TARGET_CQC = libntru-hps4096821_NR3_CQCRNG.so
CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c

SOURCES = cmov.c crypto_sort_int32.c fips202.c kem.c owcpa.c pack3.c packq.c poly.c poly_lift.c poly_mod.c poly_r2_inv.c poly_rq_mul.c poly_s3_inv.c PQCgenKAT_kem.c rng.c sample.c sample_iid.c
LIB_SOURCES_RNG = cmov.c crypto_sort_int32.c fips202.c kem.c owcpa.c pack3.c packq.c poly.c poly_lift.c poly_mod.c poly_r2_inv.c poly_rq_mul.c poly_s3_inv.c rng.c sample.c sample_iid.c
LIB_SOURCES_CQC = cmov.c crypto_sort_int32.c fips202.c kem.c owcpa.c pack3.c packq.c poly.c poly_lift.c poly_mod.c poly_r2_inv.c poly_rq_mul.c poly_s3_inv.c $(CQCRANDOM_SRC) sample.c sample_iid.c
HEADERS = api_bytes.h api.h cmov.h crypto_hash_sha3256.h crypto_sort_int32.h fips202.h kem.h owcpa.h params.h poly.h rng.h sample.h

PQCgenKAT_kem: $(HEADERS) $(SOURCES)
	$(CC) -DCRYPTO_NAMESPACE\(s\)=ntru_\#\#s -o $@ $(SOURCES) $(LDFLAGS)

$(LIB_TARGET_RNG): $(HEADERS) $(LIB_SOURCES_RNG)
	$(CC) -DCRYPTO_NAMESPACE\(s\)=ntru_\#\#s -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_RNG) $(LDFLAGS)

$(LIB_TARGET_CQC): $(HEADERS) $(LIB_SOURCES_CQC)
	$(CC) -DCRYPTO_NAMESPACE\(s\)=ntru_\#\#s -fPIC -DSMALL_STACK -shared -o $@ $(LIB_SOURCES_CQC) $(LDFLAGS)

shared: $(LIB_TARGET_RNG) $(LIB_TARGET_CQC)

.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem
	-rm -f $(LIB_TARGET_RNG)
	-rm -f $(LIB_TARGET_CQC)
