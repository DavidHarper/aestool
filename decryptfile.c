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

  error = gcry_cipher_open(&hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_OFB, 0);

  if (error != GPG_ERR_NO_ERROR)
    return error;

  error = gcry_cipher_setkey(hd, key, 16);

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
