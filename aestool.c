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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gcrypt.h>

#include "aestool.h"

#define BUFSIZE 1024

static void printUsage(FILE *fp, char *progname, const char *version) {
  fprintf(fp, "aestool : an AES encryption utility\n");
  fprintf(fp, "\n");
  fprintf(fp, "Copyright (C) 2013 David Harper at obliquity.com\n\n");
  fprintf(fp, "This program comes with ABSOLUTELY NO WARRANTY.\n\n");
  fprintf(fp, "This is free software, and you are welcome to redistribute it\n");
  fprintf(fp, "under certain conditions.  Please refer to the GNU General\n");
  fprintf(fp, "Public License V3 [http://www.gnu.org/licenses/].\n\n");

  fprintf(fp, "Usage: %s -e|-d [input filename] [output filename]\n\n", progname);

  fprintf(fp, "MANDATORY EXCLUSIVE PARAMETERS:\n");
  fprintf(fp, "\t-e\t\tEncrypt input\n");
  fprintf(fp, "\t-d\t\tDecrypt input\n");
  fprintf(fp, "\n");
  fprintf(fp, "INPUT AND OUTPUT FILES\n");
  fprintf(fp, "If no named input files are specified, the program will try to\n");
  fprintf(fp, "encrypt/decrypt data from standard input and write the result to\n");
  fprintf(fp, "standard output.  The symbol - may be used to denote standard input\n");
  fprintf(fp, "in place of an input file name or standard output in place of an\n");
  fprintf(fp, "output file name.\n");
  fprintf(fp, "\n");
  fprintf(fp, "PASSPHRASE FILE\n");
  fprintf(fp, "If the environment variable AESPASSFILE is defined and refers to a file\n");
  fprintf(fp, "which cannot be read or written except by the user, then the passphrase\n");
  fprintf(fp, "will be read from that file.\n\n");
  fprintf(fp, "Otherwise, the passphrase will be read from /dev/tty.  In this case, if\n");
  fprintf(fp, "the input file is being encrypted, the user will be prompted for the\n");
  fprintf(fp, "passphrase twice, and the operation will be abandoned if the two versions\n");
  fprintf(fp, "do not match.\n");
  fprintf(fp, "\n");
  fprintf(fp, "ENCRYPTION STRENGTH\n");
  fprintf(fp, "If the environment variable AES256 is defined, AES-256 encryption will be\n");
  fprintf(fp, "used.  Otherwise, AES-128 encryption will be used.\n\n");
  fprintf(fp, "The encryption strength is NOT stored in the output file, so it is the\n");
  fprintf(fp, "user's responsibility to remember whether AES-128 or AES-256 encryption was\n");
  fprintf(fp, "used to create any given file.\n");
  fprintf(fp, "\n");
  fprintf(fp, "This program uses libgcrypt version %s\n", (version ? version : "[unknown]"));
}

static int exists(char *filename) {
  struct stat stat_buf;
  int rc;

  rc = stat(filename, &stat_buf);

  return (rc == 0) ? 1 : 0;
}

int main(int argc, char **argv) {
  FILE *infile = NULL;
  FILE *outfile = NULL;
  unsigned char buffer[BUFSIZE];
  unsigned char IV[16];
  unsigned char key[32];

  int mode = UNKNOWN;

  char *infilename = NULL;
  char *outfilename = NULL;

  char *progname= argv[0];

  int rc;
  gcry_error_t error;
  const char *version;

  version = gcry_check_version(GCRYPT_VERSION);

  if (argc < 2) {
    printUsage(stderr, progname, version);
    return 1;
  }

  if (!version) {
    fprintf(stderr, "libgcrypt version mismatch: expected %s, found %s\n",
      GCRYPT_VERSION, version);
    return 99;
  }

  gcry_control(GCRYCTL_DISABLE_SECMEM, 0);

  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

  if (strcmp(argv[1], "-e") == 0)
    mode = ENCRYPT;
  else if (strcmp(argv[1], "-d") == 0)
    mode = DECRYPT;
  else {
    fprintf(stderr, "You must specify either -d or -e as the first command line option\n");
    printUsage(stderr, progname, version);
    return 1;
  }

  rc = getPassphrase((char *)buffer, sizeof(buffer), mode);

  switch (rc) {
  case BUFFER_TOO_SHORT:
    fprintf(stderr, "%s: passphrase was too long\n", progname);
    return 2;

  case PASSPHRASE_MISMATCH:
    fprintf(stderr, "%s: the passphrase did not match\n", progname);
    return 3;
  }

  rc = strlen((char *)buffer);

  error = createPassphraseHash(buffer, (size_t)rc, key, sizeof(key));

  if (error != GPG_ERR_NO_ERROR) {
    handleGcryptError("An error occurred whilst hashing the passphrase", error, stderr);
    return 98;
  }

  memset(buffer, '\0', sizeof(buffer));

  infilename = argc > 2 && strcmp(argv[2], "-") != 0 ? argv[2] : NULL;

  infile = (infilename != NULL) ? fopen(infilename, "rb") : stdin;

  if (infilename != NULL && infile == NULL) {
    perror("Failed to open input file");
    return 5;
  }

  outfilename = argc > 3 && strcmp(argv[3], "-") != 0 ? argv[3] : NULL;

  outfile = (outfilename != NULL) ? fopen(outfilename, "wb") : stdout;

  if (outfilename != NULL && outfile == NULL) {
    perror("Failed to open output file");
    return 6;
  }

  if (mode == ENCRYPT) {
    gcry_create_nonce(IV, sizeof(IV));
  } else {
    fread(IV, 1, sizeof(IV), infile);
  }

  error = (mode == ENCRYPT) ?
    encryptFile(infile, IV, key, outfile) :
    decryptFile(infile, IV, key, outfile);

  if (error != GPG_ERR_NO_ERROR) {
    handleGcryptError("An error occurred whilst processing the data", error, stderr);
    return 97;
  }

  return 0;
}
