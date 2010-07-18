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

  error = gcry_cipher_open(&hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_OFB, 0);

  if (error != GPG_ERR_NO_ERROR)
    return error;

  error = gcry_cipher_setkey(hd, key, 16);

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
