#include <stdio.h>

#include <xyssl/sha2.h>
#include <xyssl/aes.h>

#include "aestool.h"

int decryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile) {
  aes_context aes_ctx;

  unsigned char inbuffer[16];
  unsigned char obp[16];
  unsigned char *outbuffer = NULL;

  int eof = 0;
  int n;

  aes_set_key(&aes_ctx, key, 128);

  while (!eof) {
    n = fread(inbuffer, 1, 16, infile);

    if (n > 0 && n < 16)
      return PREMATURE_END_OF_FILE;

    if (n == 0) {
      if (outbuffer == NULL)
	return PREMATURE_END_OF_FILE;

      n = 16 - (int)outbuffer[15];

      eof = 1;
    }

    if (outbuffer != NULL)
      fwrite(outbuffer, 1, n, outfile);

    outbuffer = obp;
    
    aes_encrypt(&aes_ctx, IV, outbuffer);

    for (n = 0; n < 16; n++) {
      IV[n] = outbuffer[n];

      outbuffer[n] ^= inbuffer[n];
    }
  }

  return OK;
}
