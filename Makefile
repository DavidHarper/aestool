# Makefile for file encryption and decryption code
#
# Created by David Harper at obliquity.com

# Define compiler options
CC = gcc
CFLAGS = -c $(DEBUG)

# Definer linker options
LD = gcc
LDOPTS = $(DEBUG)
LDLIBS = -lgcrypt

AESTOOL_OBJS = aestool.o \
	getpassphrase.o \
        createpassphrasehash.o \
        getcipherstrength.o \
        handlegcrypterror.o \
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
