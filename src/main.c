/*
 * this code is currently believed to work for pandora and twitch
 * youtube do something funny that I haven't figured out yet
 * i don't regularly use anything else so that's all that's been tested
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"
#include "sys/stat.h"
#include "unistd.h"

#include "util.h"

#define DEFAULT_EXT ".mp4"
#define NAME_LEN 1024
#define PROC "/proc/"
#define MAX_PARALLEL 256
#define SLEEP "sleep 5"

/* 
 * keep a list of flash files that are known to be currently open
 */
int 
  n, 
  cursize[MAX_PARALLEL], lastsize[MAX_PARALLEL], 
  saved[MAX_PARALLEL];
char filename[MAX_PARALLEL][NAME_LEN];

int tracked (char *name) {
  int i, ret = 0;
  for (i = 0; i < n; ++i)
    if (!strcmp (name, filename[i]))
      ret = 1;
  return ret;
}

/*
 * read symlinks in /proc and add any new flash vidoes to array
 */
void scan (void) {
  struct dirent *d;
  DIR *proc = opendir (PROC); assert (proc);
  while (d = readdir (proc), d) {
    if (isdigit (d->d_name[0])) {
      DIR *fd;
      char *path = xstrcat3 (PROC, d->d_name, "/fd");
      if (fd = opendir (path), fd) {
        while (d = readdir (fd), d) {
          char *name = xstrcat3 (path, "/", d->d_name);
          char buf[NAME_LEN];

          if (readlink (name, buf, NAME_LEN - 1) > 0)
            if (1
                && strstr (buf, "Chrome") 
                && strstr (buf, getlogin ())
                && strstr (buf, "Shockwave Flash")) {
              if (n < MAX_PARALLEL && !tracked (name)) {
                struct stat st;
                if (stat (name, &st))
                  fprintf (stderr, 
                  "warning: %s: %s\n", name, strerror (errno));
                else {
                  strcpy (filename[n], name);
                  cursize[n] = st.st_size; lastsize[n] = -1;
                  saved[n] = 0;
                  ++n;
                }
              } else if (n == MAX_PARALLEL) {
                fprintf (stderr,
                  "warning: omitting %s: %s of %d\n",
                  name,
                  "exceeded max parallel downloads",
                  MAX_PARALLEL
                );
              }
            }
          xfree (name);
        }
        closedir (fd);
      }
      xfree (path);
    }
  }
  closedir (proc);
}

/*
 * update file sizes; delete files that no longer exist
 * save file to local directory if download has completed
 */
void update (void) {
  int i, j;
  for (i = 0; i < n; ++i) {
    struct stat st;
    if (cursize[i] == lastsize[i] && !saved[i]) {
      char buf[32];
      for (j = 0; ; ++j) {
        sprintf (buf, "%d%s", j, DEFAULT_EXT);
        if (stat (buf, &st)) {
          int c;
          FILE *f = fopen (filename[i], "r"), *g = fopen (buf, "w");
          if (f && g) {
            assert (f); assert (g);
            while (c = fgetc (f), c != EOF)
              fputc (c, g);
            fputc (EOF, g);
            fclose (f);
            saved[i] = 1;
            printf ("cp %s %s\n", filename[i], buf);
            break;
          } else {
            fprintf (stderr, "warning: failed to open %s, %s %s",
              filename[i], buf, "for read, write"
            );
          }
        }
      }
    } else if (stat (filename[i], &st)) {
      --n;
      printf ("delete %s\n", filename[i]);
      for (j = i; j < n; ++j) {
        strcpy (filename[j], filename[j + 1]);
        cursize[j] = cursize[j + 1];
        lastsize[j] = lastsize[j + 1];
        saved[j] = saved[j + 1];
      }
    } else if (!saved[i]) {
      printf ("update %s (%d %lu)\n", filename[i], cursize[i], st.st_size);
      lastsize[i] = cursize[i];
      cursize[i] = st.st_size;
    } else if (st.st_size != lastsize[i]) {
      printf ("new file %s (%d -> %lu)\n",
        filename[i], lastsize[i], st.st_size
      );
      saved[i] = 0;
      lastsize[i] = 0;
      cursize[i] = st.st_size;
    } else printf ("unchanged %s %d %lu\n", filename[i], cursize[i],
      st.st_size);
  }
}

int main (void) {
  for (;;) {
    scan ();
    update ();
    system (SLEEP);
  }
  exit (EXIT_SUCCESS);
}
