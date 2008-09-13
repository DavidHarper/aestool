#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aestool.h"

static int checkPassFile(struct stat *buf) {
  int myuid = getuid();
  return (myuid == buf->st_uid) && ((buf->st_mode & 0077) == 0);
}

static FILE * getPassFile() {
  const char *aespassfile = getenv("AESPASSFILE");
  struct stat pass_stat;
  int rc;

  if (aespassfile != NULL) {
    rc = stat(aespassfile, &pass_stat);

    if (rc == 0 && checkPassFile(&pass_stat))
      return fopen(aespassfile, "r");
  }

  return fopen("/dev/tty", "r");
}

int getPassphrase(char *buffer, int size, int mode) {
  FILE *infile = NULL;;
  int interactive = 0;
  char *buffer2 = NULL;
  FILE *ttyout;
  int fd = -1;
  struct termios ios;
  int k;

  infile = getPassFile();
  interactive = isatty(fileno(infile));

  if (interactive) {
    ttyout = fopen("/dev/tty", "w");
    fd = fileno(ttyout);
  }
  
  /*
   * In interactive mode, disable echo on stdout and display a prompt.
   */

  if (interactive) {
    tcgetattr(fd, &ios);

    ios.c_lflag ^= ECHO;
    tcsetattr(fd, TCSANOW, &ios);

    fprintf(stderr, "Enter passphrase> ");
  }

  fgets((char *)buffer, size, infile);

  /*
   * In interactive mode, enable the echo on stdout.
   */

  if (interactive) {
    ios.c_lflag |= ECHO;
    tcsetattr(fd, TCSANOW, &ios);

    fprintf(stderr, "\n");
  }

  /*
   * Check that the last character read from the file is a newline.
   * If it is not, then our buffer was too short.
   */

  k = strlen(buffer);

  if (buffer[k-1] != '\n')
    return BUFFER_TOO_SHORT;
  else
    buffer[k-1] = '\0';
  
  /*
   * In interactive mode, and when we are encrypting, prompt the user
   * to confirm the passphrase.  As before, we disable the echo on stdout
   * whilst the user types the passphrase, and enable it again afterwards.
   * Then we check the two versions of the passphrase.  If they do not
   * match, we return an error code.
   */

  if (mode == ENCRYPT && interactive) {
    buffer2 = malloc((size_t)size);

    ios.c_lflag ^= ECHO;
    tcsetattr(fd, TCSANOW, &ios);

    fprintf(stderr, "Enter passphrase again> ");
  
    fgets((char *)buffer2, size, infile);

    ios.c_lflag |= ECHO;
    tcsetattr(fd, TCSANOW, &ios);

    fprintf(stderr, "\n");

    k = strlen(buffer2);
    buffer2[k-1] = '\0';

    k = strcmp((char *)buffer, (char *)buffer2);

    free(buffer2);

    if (k != 0)
      return PASSPHRASE_MISMATCH;
  }

  return OK;
}
