/* getline.h -- getline replacement for windows file */
/* Jan Brittenson and placed under the GPL */

ssize_t getline(char **lineptr, size_t *n, FILE *stream);
/* compatible to *nix getline */
