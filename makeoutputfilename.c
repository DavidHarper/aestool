#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "aestool.h"

char * makeOutputFileName(char *infilename, int mode) {
  char *outfilename = NULL;
  int namelen;
  char *suffix;

  if (infilename == NULL || (mode != ENCRYPT && mode != DECRYPT))
    return NULL;

  namelen = strlen(infilename);

  if (mode == ENCRYPT) {
    outfilename = malloc(namelen + 5);
    strcpy(outfilename, infilename);
    strcat(outfilename, ".aes");
  } else {
    suffix = rindex(infilename, '.');

    if (suffix != NULL && strcmp(suffix, ".aes") == 0) {
      outfilename = strdup(infilename);
      outfilename[namelen - 4] = '\0';
    } else {
      outfilename = malloc(namelen + 5);
      strcpy(outfilename, infilename);
      strcat(outfilename, ".raw");
    }
  }

  return outfilename;    
}
