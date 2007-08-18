/*
 * AES encryption/decryption utility
 *
 * Author: David Harper at obliquity.com
 */

#include <string.h>
#include <stdlib.h>

#include <xyssl/sha2.h>
#include <xyssl/aes.h>

#include "aestool.h"

#define BUFSIZE 1024

int main(int argc, char **argv) {
  sha2_context sha_ctx;
  aes_context aes_ctx;
  FILE *infile;
  FILE *outfile;
  unsigned char buffer[BUFSIZE];
  unsigned char *IV;
  unsigned char key[16];

  int mode = UNKNOWN;

  char *infilename = NULL;
  char *outfilename;

  char *passfilename = NULL;

  char *progname= argv[0];

  int rc;

  for (argv++; *argv != NULL  && **argv == '-'; argv++) {
    if (strcmp(*argv, "-e") == 0) {
      if (mode == DECRYPT) {
	fprintf(stderr, "%s: -d and -e are mutually exclusive options\n", progname);
	return 1;
      }

      mode = ENCRYPT;
    } else if (strcmp(*argv, "-d") == 0) {
      if (mode == ENCRYPT) {
	fprintf(stderr, "%s: -d and -e are mutually exclusive options\n", progname);
	return 1;
      }

      mode = DECRYPT;
    } else if (strcmp(*argv, "-p") == 0)
      passfilename = *(++argv);
  }

  if (mode == UNKNOWN) {
    fprintf(stderr, "%s: you must specify either -e for encryption or -d for decryption\n", progname);
    return 1;
  }

  infile = (passfilename != NULL) ? fopen(passfilename, "r") : stdin;

  rc = getPassphrase(infile, (char *)buffer, sizeof(buffer), mode);

  if (infile != stdin)
    fclose(infile);

  switch (rc) {
  case BUFFER_TOO_SHORT:
    fprintf(stderr, "%s: passphrase was too long\n", progname);
    return 2;

  case PASSPHRASE_MISMATCH:
    fprintf(stderr, "%s: the passphrase did not match\n", progname);
    return 3;
  }

  rc = strlen((char *)buffer);
  
  sha2_starts(&sha_ctx, 0);
  sha2_update(&sha_ctx, buffer, rc);
  sha2_finish(&sha_ctx, key);

  memset(buffer, '\0', sizeof(buffer));

  infilename = *argv;

  infile = (infilename != NULL) ? fopen(infilename, "rb") : stdin;

  outfilename = makeOutputFileName(infilename, mode);

  outfile = (outfilename != NULL) ? fopen(outfilename, "wb") : stdout;

  if (mode == ENCRYPT) {
    IV = generateIV();
  } else {
    IV = (unsigned char *)malloc(16);
    fread(IV, 1, 16, infile);
  }

  rc = (mode == ENCRYPT) ? encryptFile(infile, IV, key, outfile) : decryptFile(infile, IV, key, outfile);

  

  return 0;
}
