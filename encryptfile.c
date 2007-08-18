#include <stdio.h>

#include <xyssl/sha2.h>
#include <xyssl/aes.h>

#include "aestool.h"

int encryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile) {
  aes_context aes_ctx;
  unsigned char inbuffer[16];
  unsigned char outbuffer[16];

  int eof = 0;
  int n;
  unsigned char pad;

  fwrite(IV, 1, 16, outfile);

  aes_set_key(&aes_ctx, key, 128);

  while (!eof) {
    n = fread(inbuffer, 1, 16, infile);

    if (n < 16) {
      pad = 16 - n;

      for (; n < 16; n++)
	inbuffer[n] = pad;

      eof = 1;
    }
    
    aes_encrypt(&aes_ctx, IV, outbuffer);

    for (n = 0; n < 16; n++) {
      IV[n] = outbuffer[n];

      outbuffer[n] ^= inbuffer[n];
    }

    fwrite(outbuffer, 1, 16, outfile);
  }

  return OK;
}
