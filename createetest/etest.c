#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define BUFSIZE 262144

#define SDATASIZE ";dataSize:"
#define SZ_SDATASIZE (sizeof(SDATASIZE) - 1)

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

int createetest(int nfiles, FILE **infiles, FILE *outfile, double baseTime, double deltaTime, int baseLayers) {
  int i;
  static char inbuf[BUFSIZE];
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
  return 0;
}
