#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include "createetest.h"

void usage(char *p) {
  fprintf(stderr, "Usage: %s -B <# base layers> -b <base time> -d <delta time> -o <output file> <file> ...\n", p);
}

int main(int argc, char *argv[]) {
  FILE **infiles=NULL;
  FILE *outfile=NULL;
  int nfiles=0;
  int baseLayers=3;
  double baseTime=-1;
  double deltaTime=-1;
  int o, i;
  while ((o=getopt(argc, argv, "B:b:d:o:")) != -1) {
    switch(o) {
    case 'B':
      baseLayers=atoi(optarg);
      break;
    case 'b':
      baseTime=atof(optarg);
      break;
    case 'd':
      deltaTime=atof(optarg);
      break;
    case 'o':
      if (!(outfile = fopen(optarg, "wb"))) {
	perror(optarg);
	return -2;
      }
      break;
    default:
      usage(argv[0]);
      return -1;
      break;
    }
  }
  if (baseTime == -1 || deltaTime == -1) {
    usage(argv[0]);
    return -1;
  }

  nfiles = argc - optind;
  infiles = (FILE **)malloc(nfiles * sizeof(FILE *));
  memset(infiles, 0, nfiles * sizeof(FILE *));
  for (i = 0; i < nfiles; i++) {
    if (!(infiles[i] = fopen(argv[i + optind], "rb"))) {
      perror(argv[i + optind]);
      return -2;
    }
  }

  if (nfiles < 2) {
    usage(argv[0]);
    return -3;
  }

  createetest(nfiles, infiles, outfile, baseTime, deltaTime, baseLayers);

  if (outfile) {
    fclose(outfile);
  }
  if (infiles) {
    for (i = 0; i < nfiles; i++) {
      if (infiles[i]) {
	fclose(infiles[i]);
	infiles[i] = NULL;
      }
    }
    free(infiles);
    infiles = NULL;
  }
  return 0;
}
