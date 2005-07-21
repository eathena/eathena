// DRIVER.CC - test driver for the C++/object oriented translation and 
//             modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug 

// This translation/ modification is provided "as is," without express or 
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think 
// it does; (2) that this translation/ modification is accurate; or (3) that 
// this software is "merchantible."  (Language for this disclaimer partially 
// copied from the disclaimer below).

/* based on:

   MDDRIVER.C - test driver for MD2, MD4 and MD5

  Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "md5.hh"


// Length of test block, number of test blocks.

#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

static void  MD5_timeTrial (void);
static void  MD5_testSuite (void);
static void  MD5_file (char *);
static void  MD5_filter (void);
static void  MD5_string (unsigned char *string);
static char *MD5_usage (void);

// Main driver.

int main (int argc, char *argv[]){

  int i;

  if (argc > 1)
    for (i = 1; i < argc; i++)
      if (argv[i][0] == '-' && argv[i][1] == 's')
	MD5_string ( (unsigned char *) argv[i] + 2);
      else if (strcmp (argv[i], "-t") == 0)
	MD5_timeTrial ();
      else if (strcmp (argv[i], "-x") == 0)
	MD5_testSuite ();
      else if (strcmp (argv[i], "-h") == 0)
	cout << MD5_usage()<< flush;
      else if (strcmp (argv[i], "-help")==0)
	cout << MD5_usage()<< flush;
      else if (argv[i][0] == '-'){
	cerr << argv[i] << " is an unknown option.\n" << MD5_usage()<< flush;
	exit (1);
      }
      else
	MD5_file (argv[i]);
  else
    MD5_filter ();

  return (0);
}


// Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte blocks.

static void MD5_timeTrial ()
{
  MD5 context;
  time_t endTime, startTime;
  unsigned char block[TEST_BLOCK_LEN];
  unsigned int i;


  cout << "MD5 time trial. Digesting "<< TEST_BLOCK_LEN << " ";
  cout << TEST_BLOCK_COUNT << "-byte blocks ...";

  // Initialize block
  for (i = 0; i < TEST_BLOCK_LEN; i++)
    block[i] = (unsigned char)(i & 0xff);

  // Start timer
  time (&startTime);

  // Digest blocks
  for (i = 0; i < TEST_BLOCK_COUNT; i++)
    context.update (block, TEST_BLOCK_LEN);

  context.finalize();

  // Stop timer
  time (&endTime);

  cout << " done" << endl;

  cout << "Digest = " << context << endl;

  cout << "Time = "<< (long)(endTime-startTime) << " seconds" << endl;

  cout << "Speed = ";
  cout << (long)TEST_BLOCK_LEN * (long)TEST_BLOCK_COUNT/(endTime-startTime);
  cout << "bytes/second" <<endl;

}

// Digests a reference suite of strings and prints the results.

static void MD5_testSuite ()
{
  cout << "MD5 test suite:" << endl;

  MD5_string ( (unsigned char*) "");
  MD5_string ( (unsigned char*) "a");
  MD5_string ( (unsigned char*) "abc");
  MD5_string ( (unsigned char*) "message digest");
  MD5_string ( (unsigned char*) "abcdefghijklmnopqrstuvwxyz");
  MD5_string
 ( (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  MD5_string
 ( (unsigned char*) "1234567890123456789012345678901234567890\
1234567890123456789012345678901234567890");
}


// Digests a file and prints the result.

static void MD5_file (char *filename){

  ifstream file(filename);

  if (!file)
    cerr << filename <<" can't be opened" << endl;
  else {
    MD5 context(file);
    cout <<  "MD5 ("  << filename <<  ") = "  <<  context << endl;
  }
}

// Digests the standard input and prints the result.

static void MD5_filter ()
{

  MD5 context(cin);  // ie. run istream version of MD5 on cin.
                     // Could also run file version of MD5 on stdin.

  cout << context << endl;
}



// Digests a string and prints the result.

void MD5_string (unsigned char *string){

  MD5 context;
  unsigned int len = strlen ( (char *)string);

  context.update   (string, len);
  context.finalize ();

  cout << "MD5 (\"" << (char *)string << "\") = " << context <<endl;
}



static char *MD5_usage(){

  return "\n\
  MD5\n\n\
USAGE:\n\n\
  MD5 [-sstring] [-t] [-x] [-h] [-help] [filename]\n\n\
Arguments (may be any combination):\n\
  -sstring - digests string\n\
  -t       - runs time trial\n\
  -x       - runs test script\n\
  -h       - displays this message\n\
  -help    - displays this message\n\
  filename - digests file\n\
  (none)   - digests standard input\n\n";
}



