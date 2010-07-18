#include <stdio.h>

#include <gcrypt.h>

void handleGcryptError(const char * message, gcry_error_t error, FILE *fp) {
  fprintf(fp, "%s : %s, %s\n", message, gcry_strsource(error), gcry_strerror(error));
}
