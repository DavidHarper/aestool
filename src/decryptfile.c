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

#include <stdio.h>
#include <errno.h>

#include <gcrypt.h>

#include "aestool.h"

gcry_error_t decryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile) {
  gcry_error_t error;
  gcry_cipher_hd_t hd;

  unsigned char inbuffer[16];
  unsigned char obp[16];
  unsigned char *outbuffer = NULL;

  int eof = 0;
  int n;

  int strength = getCipherStrength();

  int algorithm = strength == 256 ? GCRY_CIPHER_AES256 : GCRY_CIPHER_AES128;

  size_t keylength = strength == 256 ? 32 : 16;

  error = gcry_cipher_open(&hd, algorithm, GCRY_CIPHER_MODE_OFB, 0);

  if (error != GPG_ERR_NO_ERROR)
    return error;

  error = gcry_cipher_setkey(hd, key, keylength);

  if (error != GPG_ERR_NO_ERROR)
    return error;

  error = gcry_cipher_setiv(hd, IV, 16);

  if (error != GPG_ERR_NO_ERROR)
    return error;

  while (!eof) {
    n = fread(inbuffer, 1, 16, infile);

    if (n > 0 && n < 16)
      return gcry_error_from_errno(EPIPE);

    if (n == 0) {
      if (outbuffer == NULL)
	return gcry_error_from_errno(EINVAL);

      n = 16 - (int)outbuffer[15];

      eof = 1;
    }

    if (outbuffer != NULL)
      fwrite(outbuffer, 1, n, outfile);

    outbuffer = obp;
    
    error = gcry_cipher_decrypt(hd, outbuffer, 16, inbuffer, 16);

    if (error != GPG_ERR_NO_ERROR)
      return error;
  }

  return GPG_ERR_NO_ERROR;
}
