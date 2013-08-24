/***********************************************************************
aestool : an AES encryption utility

Copyright (C) 2013 David Harper at obliquity.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see [http://www.gnu.org/licenses/].

***********************************************************************/

#ifndef _AESTOOL_H

#include <stdio.h>

#include <gcrypt.h>

/*
 * Encryption mode values
 */

#define UNKNOWN 0
#define ENCRYPT 1
#define DECRYPT 2

/*
 * Error codes
 */

#define OK 0
#define BUFFER_TOO_SHORT 1
#define PASSPHRASE_MISMATCH 2
#define PREMATURE_END_OF_FILE 3

/*
 * Function prototypes
 */

int getPassphrase(char *buffer, int size, int mode);

int getCipherStrength();

gcry_error_t createPassphraseHash(char *buffer, size_t buffer_size, unsigned char *key, size_t key_size);

gcry_error_t encryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile);

gcry_error_t decryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile);

void handleGcryptError(const char * message, gcry_error_t error, FILE *fp);

#define _AESTOOL_H
#endif
