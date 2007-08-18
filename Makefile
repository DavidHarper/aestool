# Makefile for file encryption and decryption code
#
# Created by David Harper at obliquity.com

# Location of xyssl.com crypto library
XYSSL = xyssl
XYSSL_INC = $(XYSSL)/include
XYSSL_LIB = $(XYSSL)/library

# Define compiler options
CC = gcc
CFLAGS = -c -I $(XYSSL_INC) $(DEBUG)

# Definer linker options
LD = gcc
LDOPTS = -L$(XYSSL_LIB) $(DEBUG)
LDLIBS = -lxyssl

AESTOOL_OBJS = aestool.o \
	getpassphrase.o \
	generateIV.o \
	makeoutputfilename.o \
	encryptfile.o \
	decryptfile.o

all: aestool

aestool: $(AESTOOL_OBJS)
	$(LD) -o $@ $(LDOPTS) $(AESTOOL_OBJS) $(LDLIBS)

clean:
	/bin/rm -f *.o aestool aestool.tgz

tarball: aestool.tgz

aestool.tgz:
	tar zcvf $@ *.c *.h Makefile
