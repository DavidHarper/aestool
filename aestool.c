/*
 * AES encryption/decryption utility
 *
 * Author: David Harper at obliquity.com
 */

#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <xyssl/sha2.h>
#include <xyssl/aes.h>

#include "aestool.h"

#define BUFSIZE 1024

static void printUsage(FILE *fp, char *progname) {
  fprintf(fp, "Usage: %s -e|-d [optional parameters] [filename]\n\n", progname);
  fprintf(fp, "MANDATORY EXCLUSIVE PARAMETERS:\n");
  fprintf(fp, "\t-e\t\tEncrypt input\n");
  fprintf(fp, "\t-d\t\tDecrypt input\n");
  fprintf(fp, "\n");
  fprintf(fp, "If no named input files are specified, the program will try to\n");
  fprintf(fp, "encrypt/decrypt data from standard input and write the result to\n");
  fprintf(fp, "standard output.  When the program is run in this mode, the use of\n");
  fprintf(fp, "the -p option is strongly encouraged.\n");
  fprintf(fp, "\n");
  fprintf(fp, "OPTIONAL PARAMETERS:\n");
  fprintf(fp, "\t-p filename\tRead passphrase from this file\n");
  fprintf(fp, "\n");
  fprintf(fp, "If this parameter is not specified, the program will try to\n");
  fprintf(fp, "read the passphrase from the standard input.  If the program\n");
  fprintf(fp, "is being used in pipe mode, this will almost certainly fail.\n");
  fprintf(fp, "\n");
  fprintf(fp, "\t-f\t\tExisting files may be overwritten if there is a name clash\n");
  fprintf(fp, "\n");
  fprintf(fp, "EXCLUSIVE OPTIONAL PARAMETERS:\n");
  fprintf(fp, "\t-s suffix\tAppend this suffix to create output file names\n");
  fprintf(fp, "\t-S suffix\tReplace suffix of input file name with this suffix\n\t\t\tto create output file name\n");
  fprintf(fp, "\t-r\t\tRemove the suffix of the input file name to create output\n\t\t\tfile name\n");
  fprintf(fp, "\n");
  fprintf(fp, "The default behaviour with named input files is to create\n");
  fprintf(fp, "output file names by appending .aes when encrypting and .raw\n");
  fprintf(fp, "when decrypting.\n");
  fprintf(fp, "\n");
  fprintf(fp, "In all cases, the operation will be abandoned if the output\n");
  fprintf(fp, "file already exists, UNLESS the -f options has been specified,\n");
  fprintf(fp, "in which case, the file will be overwritten without a warning.\n");
}

static int exists(char *filename) {
  struct stat stat_buf;
  int rc;

  rc = stat(filename, &stat_buf);

  return (rc == 0) ? 1 : 0;
}

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

  int suffixmode = APPEND;
  int noclobber = 1;

  char *suffix = NULL;

  char *passfilename = NULL;

  char *progname= argv[0];

  int rc;

  for (argv++; *argv != NULL  && **argv == '-'; argv++) {
    if (strcmp(*argv, "-e") == 0) {
      if (mode == DECRYPT) {
	fprintf(stderr, "%s: -d and -e are mutually exclusive options\n\n", progname);
	printUsage(stderr, progname);
	return 1;
      }

      mode = ENCRYPT;
    } else if (strcmp(*argv, "-d") == 0) {
      if (mode == ENCRYPT) {
	fprintf(stderr, "%s: -d and -e are mutually exclusive options\n\n", progname);
	printUsage(stderr, progname);
	return 1;
      }

      mode = DECRYPT;
    } else if (strcmp(*argv, "-p") == 0)
      passfilename = *(++argv);
    else if (strcmp(*argv, "-s") == 0) {
      suffixmode = APPEND;
      suffix = *(++argv);
    } else if (strcmp(*argv, "-S") == 0) {
      suffixmode = REPLACE;
      suffix = *(++argv);
    } else if (strcmp(*argv, "-r") == 0)
      suffixmode = REMOVE;
    else if (strcmp(*argv, "-f") == 0)
      noclobber = 0;;
  }

  if (mode == UNKNOWN) {
    printUsage(stderr, progname);
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

  outfilename = makeOutputFileName(infilename, mode, suffixmode, suffix);

  if (noclobber && exists(outfilename)) {
    fprintf(stderr, "%s: output file %s already exists.  Use -f to force overwriting\n", progname, outfilename);
    return 4;
  }

  outfile = (outfilename != NULL) ? fopen(outfilename, "wb") : stdout;

  if (mode == ENCRYPT) {
    IV = generateIV();
  } else {
    IV = (unsigned char *)malloc(16);
    fread(IV, 1, 16, infile);
  }

  rc = (mode == ENCRYPT) ?
    encryptFile(infile, IV, key, outfile) :
    decryptFile(infile, IV, key, outfile);

  return 0;
}
