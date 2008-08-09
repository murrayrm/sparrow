/*
 * loadmat - C language routine to load a matrix from a MAT-file.
 *
 * * Here is an example that uses 'loadmat' to load a matrix from a MAT-file:
 *
 *	FILE *fp;
 *	char name[20];
 *	int type, mrows, ncols, imagf;
 *	double *xr, *xi;
 *	fp = fopen("foo.mat","rb");
 *	loadmat(fp, &type, name, &mrows, &ncols, &imagf, &xr, &xi);
 *      fclose(fp);
 *	free(xr);
 *	if (imagf) free(xi);
 *
 * The 'loadmat' routine returns 0 if the read is successful and 1 if
 * and end-of-file or read error is encountered.  'loadmat' can be called
 * repeatedly until the file is exhausted and an EOF is encountered.
 *
 * Return values:
 *    1 = EOF
 *    0 = OK
 *   -1 = Error
 * 
 * Works for level 1.0 mat-files, as described in the external reference guide.
 *
 * Author Michael Kantner, 2-21-95
 * Based loosely upon work by Eric Wemhoff in July 1994
 * Uses the calling format of loadmat.c, an old matlab routine.
 *
 * Copyright (c) 2008 by California Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the California Institute of Technology nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CALTECH
 * OR THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include <stdio.h>

#ifdef MSDOS
#include <alloc.h>
#else
//#include <malloc.h>
#endif

#include <stdlib.h>

/*#define LM_DEBUG*/

long convert_long(int dataformat, int desformat, long value);
double convert_double(int dataformat, int desformat, double value);

int getformat(long typeval);
/* These are the values assigned by matlab */
#define DF_LITTLEENDIAN    0
#define DF_BIGENDIAN       1
#define DF_VAXDFLOAT       2
#define DF_VAXGFLOAT       3
#define DF_CRAY            4
char *df_types[]={"little endian","big endian","Vax D-float",
		"Vax G-float","Cray"};

int loadmat(FILE *fp, int *type, int *mrows, int *ncols, 
            int *imagf, char *pname, double **preal, double **pimag)
{
  int mn, namelen, i;
  int dataformat, desformat, datasize, datatype;
  long ltype, lmrows, lncols, limagf, lnamelen;
  double temp;

#ifdef MSDOS
  desformat = DF_LITTLEENDIAN;
#else
  desformat = DF_BIGENDIAN;
#endif

  /*
   * Get the matrix size/type information.
   */

#ifdef LM_DEBUG
  printf("    In loadmat.c\n");
#endif

  /* Read the type of the matrix */
  if (fread(&ltype,    sizeof(long), 1, fp) !=1) return 1;
  if (fread(&lmrows,   sizeof(long), 1, fp) !=1) return -1;
  if (fread(&lncols,   sizeof(long), 1, fp) !=1) return -1;
  if (fread(&limagf,   sizeof(long), 1, fp) !=1) return -1;
  if (fread(&lnamelen, sizeof(long), 1, fp) !=1) return -1;
  
#ifdef LM_DEBUG
  printf("  Unconverted Info:  Type of matrix is %i\n",ltype);
  printf("    Rows: %i, Cols: %i, imag: %i, namelen: %i\n",lmrows,lncols,
	 limagf,lnamelen);
  fflush(stdout);
#endif

  dataformat = getformat(ltype);
  
#ifdef LM_DEBUG
  printf("    In loadmat.c: got info\n");
  printf("  File is %s format.\n",df_types[dataformat]);
  printf("  Computer is %s format.\n",df_types[desformat]);
#endif

  /* Convert to desired data type */
  *type   = convert_long(dataformat,desformat,ltype);
  *mrows  = convert_long(dataformat,desformat,lmrows);
  *ncols  = convert_long(dataformat,desformat,lncols);
  *imagf  = convert_long(dataformat,desformat,limagf);
  namelen = convert_long(dataformat,desformat,lnamelen);

#ifdef LM_DEBUG
  printf("  Converted Info:  Type of matrix is %i\n",*type);
  printf("    Rows: %i, Cols: %i, imag: %i, namelen: %i\n",*mrows,*ncols,
	 *imagf,namelen);
  fflush(stdout);
  printf("Press ret to continue\n");getchar();
#endif

  mn = *mrows * *ncols;

  /*
   * Get matrix name from file
   */
  if (fread(pname, sizeof(char), namelen, fp) != namelen) {
    printf("Error reading name (%i chars)!\n",namelen);
    return(-1);
  }

#ifdef LM_DEBUG
  printf("    In loadmat.c: got name %s\n",pname);
#endif

  /* 
   * Now process the matrix format.
   */

  if (*type/1000 != dataformat){
    printf("\nError: Unknown or unexpected number format (M) ");
    printf("(expected %d, got %d)\n",dataformat, *type/1000);
    return -1;
  }
  if ((*type%1000)/100 != 0){
    printf("\nError: Unknown or unexpected number format (O) ");
    printf("(expected 0, got %d)\n",(*type%1000)/100);
    return -1;
  }
  datasize = (*type%100)/10;
  if ( (datasize < 0) || (datasize > 5) ){
    printf("\nError: Unexpected data size value (P) of %i\n",datasize);
    return -1;
  }
  datatype = (*type % 10);
  if ( (datatype < 0) || (datatype > 2) ){
    printf("\nError: Unexpected data type value (T) of %i\n",datatype);
    return -1;
  }

  if (datatype == 2){
    printf("\nError:  sparse matrices are not supported.\n");
    return -1;
  }

  if (datasize != 0){
    printf("\nError:  only 8 bit doubles can be loaded.\n");
    return -1;
  }

  /*
   * Get Real part of matrix from file
   */
#ifdef LM_DEBUG
  printf("    In loadmat.c: getting real part.\n");
#endif

  if (mn <= 0){  /* No data for the matrix! */
    printf("Warning:  loadmat.c: Null matrix\n");
    *preal=NULL;
    *pimag=NULL;
    return 0;
  }

  /*  So we now know that the matrix has elements */
  if ((*preal = (double *) malloc(mn*sizeof(double)))==NULL) {
    printf("\nError: Variable too big to load\n");
    return -1;
  }

#ifdef LM_DEBUG
  printf("    In loadmat.c: malloced the real part.\n");
#endif

  for(i=0; i<mn; i++){
#ifdef LM_DEBUG
    printf("entry %3i: ",i);
#endif
    if (fread(&temp, sizeof(double), 1, fp) != 1) {
      printf("\nError: Failed to read real part of matrix\n");
      free(*preal); *preal=NULL; return -1;
    }
#ifdef LM_DEBUG
    printf("%12.4g, ",temp);
#endif

    *(*preal + i) = convert_double(dataformat,desformat,temp);

#ifdef LM_DEBUG
    printf("%12.4g\n",*(*preal + i));
#endif
  }

#ifdef LM_DEBUG  
  printf("    In loadmat.c: got real part\n");
#endif

  /* If there is no imaginary part, we are done! */
  if (*imagf == 0) return 0;

  /* Otherwise get the imaginary part */
  if ((*pimag = (double *) malloc(mn*sizeof(double)))==NULL) {
    printf("\nError: Variable too big to load\n");
    free(*preal);preal = NULL;
    return -1;
  }
  for(i=0; i<mn; i++){
    if (fread(&temp, sizeof(double), 1, fp) != 1) {
      printf("\nError: Failed to read imaginary part of matrix\n");
      free(*preal); free(*pimag); *preal = *pimag = NULL; return -1;
    }

    *(*pimag+i) = convert_double(dataformat,desformat,temp);
  }

#ifdef LM_DEBUG
  printf("    In loadmat.c: got imag part (if it exists)\n");
#endif
  
  return(0);
}

long convert_long(int dataformat, int desformat, long value){
  if (dataformat == desformat) return value;

  /* We are on a Little Endian machine (PC) */
  if (desformat == DF_LITTLEENDIAN){
    if (dataformat == DF_BIGENDIAN)
      return (value&0xff000000)>>24 | (value&0x00ff0000)>>8 |
	(value&0x0000ff00)<<8 | (value&0x000000ff)<<24;
  }

  /* We are on a Big Endian machine (Sun) */
  if (desformat == DF_BIGENDIAN){
    if (dataformat == DF_LITTLEENDIAN)
      return (value&0xff000000)>>24 | (value&0x00ff0000)>>8 |
	(value&0x0000ff00)<<8 | (value&0x000000ff)<<24;
  }

  return value;
}

double convert_double(int dataformat, int desformat, double value){
  if (dataformat == desformat) return value;

  /* We are on a Little Endian machine (PC) */
  if (desformat == DF_LITTLEENDIAN){
    if (dataformat == DF_BIGENDIAN){
      register int i;
      double lvalue;
      char *cp;

      cp=(char*)(&value) + 7;
      for(i=0; i<8; i++)
	*((char *)(&lvalue)+i) = *(cp-i);
      return lvalue;
    }
  }

  /* We are on a Big Endian machine (Sun) */
  if (desformat == DF_BIGENDIAN){
    if (dataformat == DF_LITTLEENDIAN){
      register int i;
      double lvalue;
      char *cp;

      cp=(char*)(&value) + 7;
      for(i=0; i<8; i++)
	*((char *)(&lvalue)+i) = *(cp-i);
      return lvalue;
    }
  }

  return value;
}

/* 
 * This routine is dependant upon machine architecture!    
 */
int getformat(long type){
#ifdef MSDOS
  /* If the first 2 bytes of (long int)type are nonzero and the last
     2 are zero, then assume the number format of the file is big 
     endian.  First word==0 && last word nonzero => little endian.
     (Assuming we live in a world of only big and little endian)
     Both == 0 => little endian, because that means the type field
     was zero. If the type field, read in the determined format, is not
     in agreement, print an error. */
  if( (type & 0xffff0000) && !(type & 0x0000ffff))
    return  DF_BIGENDIAN;           /* big endian */
  else if( (type & 0xffff0000)==0 )
    return  DF_LITTLEENDIAN;	/* little endian */
  else{
    printf("\nError: Can't determine number format\n");
    return -1;
  }
#else
  /* Assume we are on a sun */
  if (type == 0) return DF_LITTLEENDIAN;
  if (type & 0xffff0000) return DF_LITTLEENDIAN;
  return DF_BIGENDIAN;
#endif  

}
