CC ?= /usr/bin/cc
CFLAGS += -Wall -Wextra -march=native -mtune=native -O3 -fPIC -fomit-frame-pointer
CFLAGS += -DMODE=1
NISTFLAGS += -march=native -mtune=native -O3 -fomit-frame-pointer
NISTFLAGS += -DMODE=1
CQCRANDOM_SRC = ../../../../../cqcrandom/cqcrandom.c
SOURCES = sign.c polyvec.c poly.c packing.c ntt.c reduce.c rounding.c fips202.c
HEADERS = config.h api.h params.h sign.h polyvec.h poly.h packing.h ntt.h \
  reduce.h rounding.h symmetric.h fips202.h



all: PQCgenKAT_sign test/test_vectors test/test_dilithium

PQCgenKAT_sign: PQCgenKAT_sign.c rng.c $(SOURCES) rng.h $(HEADERS)
	$(CC) $(NISTFLAGS) $< rng.c $(SOURCES) -o $@ -lcrypto

test/test_vectors: test/test_vectors.c rng.c $(SOURCES) rng.h $(HEADERS)
	$(CC) $(NISTFLAGS) $< rng.c $(SOURCES) -o $@ -lcrypto

test/test_dilithium: test/test_dilithium.c randombytes.c test/cpucycles.c \
  test/speed.c $(SOURCES) randombytes.h test/cpucycles.h test/speed.h $(HEADERS)
	$(CC) $(CFLAGS) $< randombytes.c test/cpucycles.c test/speed.c \
	  $(SOURCES) -o $@

test/test_mul: test/test_mul.c randombytes.c test/cpucycles.c test/speed.c \
  $(SOURCES) randombytes.h test/cpucycles.h test/speed.h $(HEADERS)
	$(CC) $(CFLAGS) -UDBENCH $< randombytes.c test/cpucycles.c \
	  test/speed.c $(SOURCES) -o $@

LDFLAGS = -lssl -L/usr/local/Cellar/openssl@1.1/1.1.1d/lib -lcrypto

libdilithium-1_NR2_CQCRNG.so:  $(CQCRANDOM_SRC) $(SOURCES) $(HEADERS)
	$(CC) -fPIC  -DSMALL_STACK -shared -o $@ $(SOURCES) $(CQCRANDOM_SRC) $(LDFLAGS)

shared: libdilithium-1_NR2_CQCRNG.so



.PHONY: clean

clean:
	rm -f *~ test/*~
	rm -f PQCgenKAT_sign
	rm -f test/test_vectors
	rm -f test/test_dilithium
	rm -f test/test_mul
	rm -f libdilithium-1_NR2_CQCRNG.so
