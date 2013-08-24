# aestool : an AES encryption utility
#
# Copyright (C) 2013 David Harper at obliquity.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see [http://www.gnu.org/licenses/].

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
	/bin/rm -f *.o aestool aestool.tgz *~

tarball: aestool.tgz

aestool.tgz:
	tar zcvf $@ *.c *.h Makefile LICENSE
