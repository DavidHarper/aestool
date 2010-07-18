#ifndef _AESTOOL_H

#include <stdio.h>

#include <gcrypt.h>

/*
 * Encryption mode values
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

int getPassphrase(char *buffer, int size, int mode);

gcry_error_t createPassphraseHash(char *buffer, size_t buffer_size, unsigned char *key, size_t key_size);

gcry_error_t encryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile);

gcry_error_t decryptFile(FILE *infile, unsigned char *IV, unsigned char *key, FILE *outfile);

void handleGcryptError(const char * message, gcry_error_t error, FILE *fp);

#define _AESTOOL_H
#endif
