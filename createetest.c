#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>

#define BUFSIZE 262144

#define SDATASIZE ";dataSize:"
#define SZ_SDATASIZE (sizeof(SDATASIZE) - 1)

void usage(char *p) {
  fprintf(stderr, "Usage: %s -B <# base layers> -b <base time> -d <delta time> -o <output file> <file> ...\n", p);
}

void handleImage(char *dataSizeLine, FILE *infile, FILE *outfile, int doout) {
  static char inbuf[BUFSIZE];
  int pngSize;

  // Print the dataSize line to the outfile
  if (doout) {
    fputs(dataSizeLine, outfile);
  }
  
  // Insert a NUL after the last digit, by searching backwards.
  while (strlen(dataSizeLine) && !isdigit(dataSizeLine[strlen(dataSizeLine) - 1])) {
    dataSizeLine[strlen(dataSizeLine) - 1] = '\0';
  }
  
  // Unnecessary sanity check that will be incredibly important if left out
  if (strlen(dataSizeLine) <= SZ_SDATASIZE) {
    fprintf(stderr, "This cannot happen #1: [%s]\n", dataSizeLine);
    exit(-999);
  }
	  
  // Read the size of the PNG data
  pngSize = atoi(&dataSizeLine[SZ_SDATASIZE]);

  // {{
  fgets(inbuf, BUFSIZE, infile);
  if (strncmp(inbuf, "{{", 2)) {
    fprintf(stderr, "dataSize not followed by {{\n");
    exit(-998);
  }
  if (doout) {
    fputs(inbuf, outfile);
  }
	  
  // PNG data
  fread(inbuf, 1, pngSize, infile);
  if (doout) {
    fwrite(inbuf, 1, pngSize, outfile);
  }

  // There's a newline after the PNG file.
  fgets(inbuf, BUFSIZE, infile);
  if (doout) {
    fputs("\n", outfile);
  }
	  
  // }}
  fgets(inbuf, BUFSIZE, infile);
  if (strncmp(inbuf, "}}", 2)) {
    fprintf(stderr, "dataSize not followed by {{\n");
    exit(-998);
  }
  if (doout) {
    fputs(inbuf, outfile);
  }
}

int main(int argc, char *argv[]) {
  FILE **infiles=NULL;
  FILE *outfile=NULL;
  int nfiles=0;
  int baseLayers=3;
  double baseTime=-1;
  double deltaTime=-1;
  int o, i;
  static char inbuf[BUFSIZE];
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

  while (fgets(inbuf, BUFSIZE, infiles[0])) {
    if (strncmp(inbuf, SDATASIZE, SZ_SDATASIZE) != 0) {
      fputs(inbuf, outfile);
    } else {
      handleImage(inbuf, infiles[0], outfile, 1);
      
      fgets(inbuf, BUFSIZE, infiles[0]);
      if (strncmp(inbuf, "M106 ", 5)) {
	fprintf(stderr, "Unknown file structure.\n");
	return -997;
      }
      fputs(inbuf, outfile);
      
      fgets(inbuf, BUFSIZE, infiles[0]);
      if (strncmp(inbuf, "G4 ", 3)) {
	fprintf(stderr, "Unknown file structure.\n");
	return -997;
      }

      if (baseLayers) {
	fputs(inbuf, outfile);
	for (i = 1; i < nfiles; i++) {
	  while (fgets(inbuf, BUFSIZE, infiles[i])) {
	    if (!strncmp(inbuf, SDATASIZE, SZ_SDATASIZE)) {
	      handleImage(inbuf, infiles[i], outfile, 0);
	      break;
	    }
	  }
	}
	baseLayers--;
      } else {
	fprintf(outfile, "G4 S%.2lf;\n", baseTime);
	for (i = 1; i < nfiles; i++) {
	  // scan to next image
	  while (fgets(inbuf, BUFSIZE, infiles[i])) {
	    if (!strncmp(inbuf, SDATASIZE, SZ_SDATASIZE)) {
	      // Trick the FHD into loading the next image
	      fprintf(outfile, "M106 S0;\n");
	      fprintf(outfile, "G1 Z0 F150;\n");
	      handleImage(inbuf, infiles[i], outfile, 1);
	      fprintf(outfile, "M106 S255;\n");
	      fprintf(outfile, "G4 S%.2lf;\n", deltaTime);
	      break;
	    }
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
