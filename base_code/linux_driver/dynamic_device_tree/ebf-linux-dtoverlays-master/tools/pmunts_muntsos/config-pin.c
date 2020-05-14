// BeagleBone GPIO Pin Configurator
// Compatible with the config-pin script available elsewhere

// Copyright (C)2018, Philip Munts, President, Munts AM Corp.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>

#define NAMES		"/sys/firmware/devicetree/base/ocp/%s_pinmux/pinctrl-names"
#define STATE		"/sys/devices/platform/ocp/ocp:%s_pinmux/state"
#define DELIMITERS	" \t\r\n"

static void fixup_pin_name(char *s)
{
  for (; *s != 0; s++)
  {
    unsigned char c = *s;

    if ((c >= 'a') && (c <= 'z'))
      *s = toupper(c);
    else if (c == '.')
      *s = '_';
  }
}

static void ListModes(char *pin)
{
  char filename[MAXPATHLEN];
  int nf;
  char buf[256];
  ssize_t len;
  int i;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  // Open the modes list file

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), NAMES, pin);

  nf = open(filename, O_RDONLY);
  if (nf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the modes list file

  len = read(nf, buf, sizeof(buf));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: read() from %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Display the available modes

  printf("\nAvailable modes for %s are:", pin);

  for (i = 0; i < len; i++)
  {
    putchar(' ');
    printf(buf + i);
    i += strlen(buf + i);
  }

  puts("\n");
  fflush(stdout);

  // Close the modes list file

  close(nf);
}

static void QueryMode(char *pin)
{
  char filename[MAXPATHLEN];
  int sf;
  char buf[256];
  ssize_t len;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), STATE, pin);

  // Open the current mode file

  sf = open(filename, O_RDONLY);
  if (sf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the current mode file

  len = read(sf, buf, sizeof(buf));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: read() from %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Display the current mode

  printf("\nCurrent mode for %s is:     %s\n", pin, buf);

  // Close the current mode file

  close(sf);
}

static void ConfigureMode(char *pin, char *mode, bool quiet)
{
  char filename[MAXPATHLEN];
  int sf;
  ssize_t len;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), STATE, pin);

  // Open the current mode file

  sf = open(filename, O_WRONLY);
  if (sf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the current mode file

  len = write(sf, mode, strlen(mode));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: write() to %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Display the current mode

  if (!quiet) printf("\nCurrent mode for %s is:     %s\n\n", pin, mode);

  // Close the current mode file

  close(sf);
}

static void ConfigFile(char *filename)
{
  FILE *cf;
  char buf[256];
  char *pin, *mode;

  // Open the specified configuration file

  cf = fopen(filename, "r");
  if (cf == NULL)
  {
    fprintf(stderr, "ERROR: fopen() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the configuration file

  while (fgets(buf, sizeof(buf), cf) != NULL)
  {
    pin = strtok(buf, DELIMITERS);
    if (pin == NULL) continue;
    if (pin[0] == '#') continue;

    mode = strtok(NULL, DELIMITERS);
    if (mode == NULL) continue;

    ConfigureMode(pin, mode, true);
  }

  // Close the configuration file

  fclose(cf);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "\nGPIO Pin Configurator\n\n");
    fprintf(stderr, "Usage: config-pin -c <filename>\n");
    fprintf(stderr, "       config-pin -l <pin>\n");
    fprintf(stderr, "       config-pin -q <pin>\n");
    fprintf(stderr, "       config-pin <pin> <mode>\n\n");
    exit(1);
  }

  if (!strcasecmp(argv[1], "-c"))
    ConfigFile(argv[2]);
  else if (!strcasecmp(argv[1], "-l"))
    ListModes(argv[2]);
  else if (!strcasecmp(argv[1], "-q"))
    QueryMode(argv[2]);
  else
    ConfigureMode(argv[1], argv[2], false);

  exit(0);
}
