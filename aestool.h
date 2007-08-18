#ifndef _AESTOOL_H

#include <stdio.h>

/*
 * Mode values
 */

#define UNKNOWN 0
#define ENCRYPT 1
#define DECRYPT 2

/*
 * Error codes
 */

#define OK 0
#define BUFFER_TOO_SHORT 1
#define PASSPHRASE_MISMATCH 2
#define PREMATURE_END_OF_FILE 3

/*
 * Function prototypes
 */

int getPassphrase(FILE *infile, char *buffer, int size, int mode);

unsigned char * generateIV();

char * makeOutputFileName(char *infilename, int mode);

int encryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile);

int decryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile);

#define _AESTOOL_H
#endif
