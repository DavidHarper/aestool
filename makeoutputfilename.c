#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "aestool.h"

char * makeOutputFileName(char *infilename, int mode) {
  char *outfilename = NULL;
  int namelen;
  char *suffix;
  struct stat stat_buf;
  int rc;

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

      rc = stat(outfilename, &stat_buf);
      if (rc == 0) {
	free(outfilename);
	outfilename = NULL;
      }
    } 

    if (outfilename == NULL) {
      outfilename = malloc(namelen + 5);
      strcpy(outfilename, infilename);
      strcat(outfilename, ".raw");
    }
  }

  return outfilename;    
}
