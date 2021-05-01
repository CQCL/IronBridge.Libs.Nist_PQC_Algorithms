CC=gcc
WARNING_FLAGS=-Wall -Wextra -Wpedantic -Werror
CFLAGS=  -fPIC -O2 -march=native $(WARNING_FLAGS) -std=gnu99 -D__LINUX__ -D__X64__ -I./sha3
#CFLAGS_DEBUG= -g -march=native $(WARNING_FLAGS) -std=gnu99 -fsanitize=address -D__LINUX__ -D__X64__ -I./sha3
CFLAGS_DEBUG= -g -march=native $(WARNING_FLAGS) -std=gnu99 -D__LINUX__ -D__X64__ -I./sha3
NISTKATFLAGS = -Wno-sign-compare -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-result
CQCRANDOM_SRC = ../../../../cqcrandom/cqcrandom.c
SHA3LIB=libshake.a
SHA3_PATH=sha3
LDFLAGS= $(SHA3_PATH)/$(SHA3LIB) 

ifeq ($(PICNIC_BUILD_BIG_ENDIAN), 1)
	    CFLAGS+=-DPICNIC_BUILD_BIG_ENDIAN
endif

SOURCES= picnic_impl.c picnic2_impl.c picnic.c lowmc_constants.c
PICNIC_OBJECTS= picnic_impl.o picnic2_impl.o picnic.o lowmc_constants.o hash.o picnic_types.o api.o tree.o
PICNIC_LIB= libpicnic.a
EXECUTABLE_EXAMPLE=example
EXECUTABLE_TESTVECTORS=create_test_vectors
EXECUTABLE_UNITTEST=unit_test
EXECUTABLE_APITEST=apitest
EXECUTABLE_NISTKAT=PQCgenKAT_sign
EXECUTABLE_BENCHMARK=bench
RNG_NISTKAT=NIST-KATs/rng.o
EXECUTABLE_KATSTEST=kats_test
EXECUTABLE_TREETEST=tree_test

all: $(SHA3LIB) $(SOURCES) $(PICNIC_LIB) $(EXECUTABLE_EXAMPLE) $(EXECUTABLE_TESTVECTORS) $(EXECUTABLE_UNITTEST) $(EXECUTABLE_APITEST) $(EXECUTABLE_KATSTEST)  $(EXECUTABLE_TREETEST) 

testVectors: $(SOURCES) $(EXECUTABLE_TESTVECTORS)

nistkat: CFLAGS+= -DUSE_NIST_KAT_INSECURE_RNG 
nistkat: LDFLAGS+=  $(RNG_NISTKAT) -lssl -lcrypto 
nistkat: clean $(RNG_NISTKAT) all $(EXECUTABLE_NISTKAT)

$(SHA3LIB):
		$(MAKE) -C $(SHA3_PATH) 

# debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: all

$(EXECUTABLE_EXAMPLE): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

$(EXECUTABLE_UNITTEST): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

$(EXECUTABLE_TREETEST): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

$(EXECUTABLE_APITEST): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

$(EXECUTABLE_TESTVECTORS): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

$(RNG_NISTKAT):
		$(CC) NIST-KATs/rng.c -c $(CFLAGS) $(NISTKATFLAGS) -INIST-KATs/ -o $@ 

$(EXECUTABLE_NISTKAT): $(RNG_NISTKAT) $(PICNIC_LIB)
		$(CC) NIST-KATs/$@.c $(CFLAGS) $(NISTKATFLAGS) -I. $(PICNIC_LIB) -o NIST-KATs/$@ $(LDFLAGS) -lssl -lcrypto

$(EXECUTABLE_BENCHMARK): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

$(EXECUTABLE_KATSTEST): $(PICNIC_LIB)
	    $(CC) $(@).c $(CFLAGS) $(PICNIC_LIB) -o $@ $(LDFLAGS)

.c.o: 
	    $(CC) -c $(CFLAGS) $< -o $@

$(PICNIC_LIB): $(PICNIC_OBJECTS)
	ar rcs $@ $^

libpicnic-l1ur_NR2_CQCRNG.so: $(PICNIC_OBJECTS)
	$(CC) -fPIC -shared -o $@ $(CQCRANDOM_SRC) $(PICNIC_OBJECTS) -lkeccak
	
shared: libpicnic-l1ur_NR2_CQCRNG.so


docs:
	doxygen docs/doxygen.cfg

.PHONY: docs

clean:
	    rm *.o 2>/dev/null || true
	    rm *.exe 2>/dev/null || true
	    rm $(EXECUTABLE_TESTVECTORS) 2>/dev/null || true
	    rm $(EXECUTABLE_EXAMPLE) 2>/dev/null || true
	    rm $(EXECUTABLE_UNITTEST) 2>/dev/null || true
	    rm $(EXECUTABLE_APITEST) 2>/dev/null || true
	    rm $(EXECUTABLE_TREETEST) 2>/dev/null || true
	    rm $(EXECUTABLE_KATSTEST) 2>/dev/null || true
	    rm $(EXECUTABLE_TESTVECTORS) 2>/dev/null || true
	    rm $(EXECUTABLE_BENCHMARK) 2>/dev/null || true
	    rm NIST-KATs/$(EXECUTABLE_NISTKAT) 2>/dev/null || true
		rm NIST-KATs/rng.o 2>/dev/null || true
		rm NIST-KATs/*.rsp 2>/dev/null || true
		rm NIST-KATs/*.req 2>/dev/null || true
		rm $(PICNIC_LIB) 2>/dev/null || true
		$(MAKE) -C $(SHA3_PATH) clean

# Run the whitespace tool 
# Install with 'sudo apt-get install uncrustify'
# Make sure you have staged your changes, so that you can easily undo changes
# made by uncrustify if necessary
whitespace:
	    uncrustify -c uncrustify.cfg -F uncrustify-file-list.txt --no-backup --replace
	
