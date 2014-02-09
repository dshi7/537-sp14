/*
 * =====================================================================================
 *
 *       Filename:  search.c
 *
 *    Description:  project 1    CS 537 Spring 14
 *
 *        Created:  01/24/2014 09:45:36 AM
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi (), dshi7@wisc.edu
 *
 * =====================================================================================
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
  char*     name;
  int       cnt_key_word;
} fileProperty;

void  getKeyWordCount ( fileProperty *file, const char *key_word ) ;

void  mergeSegments ( fileProperty* files, const int left, const int mid, const int right ) ;

void  mergeSort ( fileProperty* files, const int left, const int right ) ;

int   isInteger ( const char *str );

int main ( int argc, char *argv[] ) {

  /* check if the argv[1] is a valid integer */
  ++ argv; 
  if ( !isInteger ( *argv ) ) {
    fprintf ( stderr, "Error: %s is not a valid integer.\n", *argv );
    exit(1);
  }
  int     input_number = atoi ( *argv );

//  printf ( "input_number = %d\n", input_number );

  /* check if the number of arguments is legal */
  if ( argc-3 != input_number && argc-3 != input_number+1 ) {
    fprintf ( stderr, "Usage: search <input-number> <key-word> <input-list> <output>\n" );
    exit(1);
  }

//  printf ( "number of arguments matches.\n" );

  fileProperty*   files = (fileProperty*)calloc( input_number, sizeof(fileProperty) );
  if ( !files ) {
    fprintf ( stderr, "Malloc failed\n" );
    exit(1);
  }

  /* malloc a string for keyword */
  ++ argv; //      KEY_WORD
  char*     key_word = (char*)calloc((unsigned)strlen(*argv), sizeof(char));
  if ( !key_word ) {
    fprintf ( stderr, "Malloc failed\n" );
    exit(1);
  }
  strcpy( key_word, *argv );

  int i=0;
  while ( i < input_number ) {
    files[i].name  = *(++argv);
    getKeyWordCount ( &files[i], key_word );
    ++i;
  }

  FILE  *ofp = NULL;
  /* check if the output file is valid */
  if ( argc-3 == input_number+1 ) {
    char  output_realpath[PATH_MAX+1];
    ofp = fopen ( *(++argv), "w" );
    if (!ofp) {
      fprintf ( stderr, "Error: Cannot open file \'%s\'\n", *argv );
      exit(1);
    }
    char  *ores = realpath ( *argv, output_realpath );
    if ( !ores ) {
      fprintf ( stderr, "Error: Cannot find the real path for output file \"%s\"\n", *argv );
      exit(1);
    }
    for ( i=0; i<input_number; i++ ) {
      char input_realpath[PATH_MAX+1];
      char* ires = realpath ( files[i].name, input_realpath );
      if ( !ires ) {
        fprintf ( stderr, "Error: Cannot find the real path for input file \"%s\"\n", files[i].name );
        exit(1);
      }
      if ( strcmp ( output_realpath, input_realpath )==0 ) {
        fprintf ( stderr, "Input and output file must differ\n" );
        exit(1);
      }
    }
  }

  mergeSort ( files, 0, input_number-1 );

  /* check if write to specific file or stdout */
  for ( i=0; i<input_number; i++ )
    fprintf ( (ofp)?ofp:stdout, "%d %s\n", files[i].cnt_key_word, files[i].name );

  free ( files );

  return 0;
}

int   isInteger ( const char *str ) {
  while ( *str )
    if ( !isdigit(*str++) )
      return  0;
  return 1;
}

void  getKeyWordCount ( fileProperty *file, const char *key_word ) {

  char buf[PATH_MAX+1];
  char* res = realpath ( (*file).name, buf );
  if ( !res ) {
    fprintf ( stderr, "Error: Cannot open file \'%s\'\n", (*file).name );
    exit(1);
  }

  FILE *ifp = fopen ( (*file).name, "r" );

  /* use a long string to store the entire file */
  fseek ( ifp, 0, SEEK_END );
  unsigned  file_str_size = ftell ( ifp );
  rewind ( ifp );
  char  *file_contents = (char*)calloc( file_str_size+1, sizeof(char) );
  if ( !file_contents ) {
    fprintf ( stderr, "Malloc failed" );
    exit(1);
  }
  fread ( file_contents, sizeof(char), file_str_size, ifp );
  fclose ( ifp );
  file_contents[file_str_size] = '\0';

  /* count the matches in the long string */
  int loc = 0;
  char  *ptr_find;
  (*file).cnt_key_word = 0;
  while ( (ptr_find = strstr(file_contents + loc, key_word ))!=NULL ) {
    loc = (int)(ptr_find-file_contents)+strlen(key_word);
    ++ (*file).cnt_key_word;
  }
  free ( file_contents );
}

void  mergeSegments ( fileProperty* files, const int left, const int mid, const int right ) {

  int n1 = mid-left+1;
  int n2 = right-mid;
  fileProperty *L = calloc ( n1+1, sizeof(fileProperty) );
  fileProperty *R = calloc ( n2+1, sizeof(fileProperty) );
  if ( !L || !R ) {
    fprintf ( stderr, "Malloc failed\n" );
    exit(1);
  }

  int i,j,k;
  for ( i=0; i< n1; i++ )
    L[i] = files[left+i];
  L[n1].cnt_key_word = -1;
  for ( j=0; j< n2; j++ )
    R[j] = files[mid+1+j];
  R[n2].cnt_key_word = -1;

  for ( i=0, j=0, k=left; k <= right; k++ ) {
    if ( L[i].cnt_key_word >=R[j].cnt_key_word ) 
      files[k] = L[i++];
    else
      files[k] = R[j++];
  }
  free (L);
  free (R);
}

void  mergeSort ( fileProperty* files, const int left, const int right ) {
  if ( left >= right ) 
    return;
  int q = (left+right)/2;
  mergeSort ( files, left, q );
  mergeSort ( files, q+1, right );
  mergeSegments ( files, left, q, right );
}
