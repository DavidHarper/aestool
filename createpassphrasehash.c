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

#include<stdlib.h>

#include <gcrypt.h>

#include "aestool.h"

gcry_error_t createPassphraseHash(char *buffer, size_t buffer_size, unsigned char *key, size_t key_size) {
  gcry_md_hd_t hd;
  gcry_error_t error;
  unsigned char *result;
  int i;
  unsigned int result_size;

  result_size = gcry_md_get_algo_dlen(GCRY_MD_SHA256);

  if (key_size < result_size)
    return gcry_error(GPG_ERR_USER_1);

  error = gcry_md_open(&hd, GCRY_MD_SHA256, 0);

  if (error != GPG_ERR_NO_ERROR)
    return error;

  gcry_md_write(hd, buffer, buffer_size);

  result = gcry_md_read(hd, GCRY_MD_SHA256);

  for (i = 0; i < result_size; i++)
    key[i] = result[i];

  gcry_md_close(hd);
  
  return GPG_ERR_NO_ERROR;
}
