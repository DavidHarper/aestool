#include <stdlib.h>

int getCipherStrength() {
  char *aes256 = getenv("AES256");

  return (aes256 != NULL) ? 256 : 128;
}
