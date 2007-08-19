#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "aestool.h"

char * makeOutputFileName(char *infilename, int mode, int suffixmode, char *suffix) {
  char *outfilename = NULL;
  int namelen;
  int suffixlen;
  struct stat stat_buf;
  int rc;
  char *lastdot;

  if (infilename == NULL || (mode != ENCRYPT && mode != DECRYPT))
    return NULL;

  if (suffix == NULL)
    suffix = (mode == ENCRYPT) ? "aes" : "raw";

  namelen = strlen(infilename);
  suffixlen = strlen(suffix);
  lastdot = rindex(infilename, '.');

  if (suffixmode == APPEND || lastdot == NULL) {
    outfilename = malloc(namelen + suffixlen + 2);
    strcpy(outfilename, infilename);
    strcat(outfilename, ".");
    strcat(outfilename, suffix);
  } else if (suffixmode == REPLACE) {
    namelen -= suffixlen+1;
    outfilename = malloc(namelen + suffixlen + 2);
    strncpy(outfilename, infilename, namelen);
    outfilename[namelen] = '\0';
    strcat(outfilename, ".");
    strcat(outfilename, suffix);  
  } else if (suffixmode == REMOVE) {
    namelen -= suffixlen+1;
    outfilename = malloc(namelen + 1);
    strncpy(outfilename, infilename, namelen);
    outfilename[namelen] = '\0';
  }

  return outfilename;    
}
