#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>

#define BUFSIZE 262144

#define SDATASIZE ";dataSize:"
#define SZ_SDATASIZE (sizeof(SDATASIZE) - 1)

void usage(char *p) {
  fprintf(stderr, "Usage: %s -b <base time> -d <delta time> -o <output file> [-c <current time, default 8>] <file> ...\n", p);
}

int main(int argc, char *argv[]) {
  FILE **infiles=NULL;
  FILE *outfile=NULL;
  int nfiles=0;
  double baseTime=-1;
  double deltaTime=-1;
  int currentTime=8;
  double waitTime;
  int o, i, pngSize, ready=0, isBaselayer;
  static char inbuf[BUFSIZE];
  while ((o=getopt(argc, argv, "b:d:c:o:")) != -1) {
    switch(o) {
    case 'b':
      baseTime=atof(optarg);
      break;
    case 'd':
      deltaTime=atof(optarg);
      break;
    case 'c':
      currentTime=atoi(optarg);
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

  while (!ready) {
    if (!fgets(inbuf, BUFSIZE, infiles[0])) {
      ready = 1;
    } else {
      if (strncmp(inbuf, SDATASIZE, SZ_SDATASIZE) != 0) {
	// For normal lines, just copy to outfile and consume one line from each one
	// of the other files.
	// Note: This is going to be massively messed up if the files don't have the
	// same structure. It's way too much work to verify each line.
	fputs(inbuf, outfile);
	for (i = 1; i<nfiles; i++) {
	  if (!fgets(inbuf, BUFSIZE, infiles[i])) {
	    fprintf(stderr, "Short file #%d.\n", i);
	    return -4;
	  }
	}
      } else {
	// ;dataSize:15864
	for (i = 0; i<nfiles; i++) {
	  if (i > 0) {
	    fgets(inbuf, BUFSIZE, infiles[i]);
	  }
	  // Handle the dataSize line.
	  // First copy it to the output file
	  if (!(isBaselayer && i > 0)) {
	    fputs(inbuf, outfile);
	  }
	  
	  // Insert a NUL after the last digit, by searching backwards.
	  while (strlen(inbuf) && !isdigit(inbuf[strlen(inbuf) - 1])) {
	    inbuf[strlen(inbuf) - 1] = '\0';
	  }
	  
	  // Unnecessary sanity check that will be incredibly important if left out
	  if (strlen(inbuf) <= SZ_SDATASIZE) {
	    fprintf(stderr, "This cannot happen #1\n");
	    return -999;
	  }
	  
	  // Read the size of the PNG data
	  pngSize = atoi(&inbuf[SZ_SDATASIZE]);
	  
	  // {{
	  fgets(inbuf, BUFSIZE, infiles[i]);
	  if (!(isBaselayer && i > 0)) {
	    fputs(inbuf, outfile);
	  }
	  
	  // PNG data
	  fread(inbuf, 1, pngSize, infiles[i]);
	  if (!(isBaselayer && i > 0)) {
	    fwrite(inbuf, 1, pngSize, outfile);
	    // There's a newline after the PNG file.
	    fputs("\n", outfile);
	  }
	  
	  // There's a newline after the PNG file.
	  fgets(inbuf, BUFSIZE, infiles[i]);
	  
	  // }}
	  fgets(inbuf, BUFSIZE, infiles[i]);
	  if (!(isBaselayer && i > 0)) {
	    fputs(inbuf, outfile);
	  }
	  
	  // M106 S255;
	  fgets(inbuf, BUFSIZE, infiles[i]);
	  if (!(isBaselayer && i > 0)) {
	    fputs(inbuf, outfile);
	  }
	  
	  // G4 S8;
	  fgets(inbuf, BUFSIZE, infiles[i]);
	  // Insert a NUL after the last digit, by searching backwards.
	  while (strlen(inbuf) && !isdigit(inbuf[strlen(inbuf) - 1])) {
	    inbuf[strlen(inbuf) - 1] = '\0';
	  }
	  waitTime = atof(inbuf + 4);
	  if ((int)waitTime == currentTime) {
	    waitTime = (i == 0)?baseTime:deltaTime;
	    isBaselayer = 0;
	  } else {
	    isBaselayer = 1;
	  }
	  if (!(isBaselayer && i > 0)) {
	    fprintf(outfile, "G4 S%.2lf;\n", waitTime);
	  }

	  // ;L:2;
	  fgets(inbuf, BUFSIZE, infiles[i]);

	  // ;M106 S0;
	  fgets(inbuf, BUFSIZE, infiles[i]);
	  if (!(isBaselayer && i > 0)) {
	    fputs(inbuf, outfile);
	  }
	}
      }
    }
  }
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
