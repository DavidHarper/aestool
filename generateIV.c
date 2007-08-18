#include <stdlib.h>

#include <xyssl/sha2.h>

#include "aestool.h"

#define NWORDS 128

unsigned char * generateIV() {
  time_t ticks = time(NULL);
  long int randdata[NWORDS];
  sha2_context sha_ctx;
  int i;
  unsigned char *IV = (unsigned char *)malloc(16);
 
  srand48((long int)ticks);

  for (i = 0; i < NWORDS; i++)
    randdata[i] = lrand48();

  sha2_starts(&sha_ctx, 0);
  sha2_update(&sha_ctx, (unsigned char *)randdata, sizeof(randdata));
  sha2_finish(&sha_ctx, IV);

  return IV;
}
