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

#include <gcrypt.h>

#include "aestool.h"

gcry_error_t encryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile) {
  gcry_error_t error;
  gcry_cipher_hd_t hd;

  unsigned char inbuffer[16];
  unsigned char outbuffer[16];

  int eof = 0;
  int n;
  unsigned char pad;

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

  fwrite(IV, 1, 16, outfile);  

  while (!eof) {
    n = fread(inbuffer, 1, 16, infile);

    if (n < 16) {
      pad = 16 - n;

      for (; n < 16; n++)
	inbuffer[n] = pad;

      eof = 1;
    }
    
    error = gcry_cipher_encrypt(hd, outbuffer, sizeof(outbuffer), inbuffer, sizeof(inbuffer));

    if (error != GPG_ERR_NO_ERROR)
      return error;

    fwrite(outbuffer, 1, 16, outfile);
  }

  gcry_cipher_close(hd);

  return GPG_ERR_NO_ERROR;
}
