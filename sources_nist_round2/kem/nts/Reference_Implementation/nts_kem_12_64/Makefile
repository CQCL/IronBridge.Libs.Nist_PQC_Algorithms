CC = gcc
OPENSSLDIR = /usr/local/opt/openssl
LIBCRYPTO = $(OPENSSLDIR)/lib/libcrypto.a
CQCRANDOM = ../../../../cqcrandom

MAKELIB = libtool -static -o $@ $^
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	OPENSSLDIR = /usr
    MAKELIB = ar cru $@ $^ && ranlib $@
	CFLAGS += -DLINUX -D_POSIX_C_SOURCE=200112L
	LIBCRYPTO = -lcrypto
	#LIBS += -lbsd
endif


INCLUDES = -I. -Inist -I$(OPENSSLDIR)/include
LIBS += $(LIBCRYPTO) -ldl -L$(OPENSSLDIR) -lssl

CFLAGS += -DNIST_DRBG_AES 
CFLAGS += -march=native -fPIC -O3 -ansi -std=c99 -Wpedantic -Wall -Werror $(INCLUDES)

LIBTARGET = lib
BINTARGET = bin

_ODIR = .obj
_ODIRKAT = .objkat
ODIR = lib bin $(_ODIR) $(_ODIRKAT) 



all :  $(BINTARGET)/ntskem-12-64-test $(BINTARGET)/ntskem-12-64-kat $(BINTARGET)/ntskem-12-64-intval

default : all

_DEPS = 
DEPS = $(patsubst %,$(INCLUDEDIR)/%,$(_DEPS))


_OBJS = additive_fft.o berlekamp_massey.o ff.o keccak.o kem.o \
		m4r.o matrix_ff2.o nts_kem.o polynomial.o cqcnts.o cqcrandom.o  \
		stack.o
OBJS = $(patsubst %,$(_ODIR)/%,$(_OBJS))
OBJSKAT = $(patsubst %,$(_ODIRKAT)/%,$(_OBJS))

$(ODIR):
	mkdir -p lib
	mkdir -p bin
	mkdir -p .obj/nist
	mkdir -p .objkat/nist

$(_ODIR)/%.o: %.c $(_ODIR) $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

$(_ODIRKAT)/%.o: %.c $(_ODIRKAT) $(DEPS)
	$(CC) $(CFLAGS) -DINTERMEDIATE_VALUES=2 -c -o $@ $< 

$(LIBTARGET)/libntskem-12-64.a: $(OBJS)
	$(MAKELIB)

$(BINTARGET)/ntskem-12-64-test: test/main.c test/ntskem_test.c $(LIBTARGET)/libntskem-12-64.a 
	$(CC) $(CFLAGS) -DNTSKEM_M=12 -DNTSKEM_T=64 -o $@ $(LDFLAGS) $^ $(LIBS) 

$(BINTARGET)/ntskem-12-64-kat: nist/PQCgenKAT_kem.c $(LIBTARGET)/libntskem-12-64.a
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ $(LIBS)

$(BINTARGET)/ntskem-12-64-intval: nist/PQCgenKAT_kem.c $(OBJSKAT)
	$(CC) $(CFLAGS) -DINTERMEDIATE_VALUES=2 -o $@ $(LDFLAGS) $^ $(LIBS)


$(LIBTARGET)/libnts-1264_NR2_CQCRNG.so: $(OBJS)
		$(CC) -shared -o $@ $^ -lssl -lcrypto
		
shared:  $(LIBTARGET)/libnts-1264_NR2_CQCRNG.so

copy_cqc: 
	cp -Rf $(CQCRANDOM)/cqcrandom.h .
	cp -Rf $(CQCRANDOM)/cqcrandom.c .
	cp -Rf $(CQCRANDOM)/cqcnts.c .

.PHONY: clean

clean:
	rm -rf $(ODIR)
