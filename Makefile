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

SRC_DIR   := src
BUILD_DIR := build
EXE 	  := $(BUILD_DIR)/aestool

PACKAGE_DIR := $(BUILD_DIR)/package
PACKAGE_NAME := aestool
DEB       := $(PACKAGE_DIR)/$(PACKAGE_NAME).deb

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(subst $(SRC_DIR), $(BUILD_DIR), $(SRCS:.c=.o))

all : $(OBJS) $(EXE)

$(EXE) : $(OBJS) | $(BUILD_DIR)
	@echo "------ Make $(EXE) ------"
	rm -f $(EXE)
	$(LD) -o $@ $(LDOPTS) $(OBJS) $(LDLIBS)
	strip $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "------ Make $(@) ------"
	rm -f $@
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

package : $(DEB)

$(DEB) : $(EXE)
	@echo "------ Make $(DEB) ------"
	rm -rf $(PACKAGE_DIR)
	mkdir -p $(PACKAGE_DIR)
	cp -rp debian-package $(PACKAGE_DIR)/$(PACKAGE_NAME)
	mkdir -p $(PACKAGE_DIR)/$(PACKAGE_NAME)/usr/bin
	cp $(EXE) $(PACKAGE_DIR)/$(PACKAGE_NAME)/usr/bin/
	chmod -R 0755 $(PACKAGE_DIR)/$(PACKAGE_NAME)/usr/bin
	gzip --best -n $(PACKAGE_DIR)/$(PACKAGE_NAME)/usr/share/man/man1/aestool.1
	gzip --best -n $(PACKAGE_DIR)/$(PACKAGE_NAME)/usr/share/doc/aestool/changelog.Debian
	cd $(PACKAGE_DIR) && dpkg-deb --root-owner-group --build $(PACKAGE_NAME)

-include $(BUILD_DIR)/*.d

clean:
	rm -rf $(BUILD_DIR)/*
