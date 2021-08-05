#ifndef __CRYPTO_H__
#define __CRYPTO_H__


/*#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/filters.h"
#include "cryptopp/ccm.h"
#include "cryptopp/aes.h"*/
#include <iostream>
#include <sys/time.h>
#include "PBC.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
using namespace std;

#define KEYLENGTH 16
#define WM_ALGO "cox"
#define NEED_INPUT 0
#define BLIND 0

unsigned char* encrypt(unsigned char ** key, unsigned char** iv, unsigned char * plaintext, int len);
unsigned char* decrypt(unsigned char* key, unsigned char* iv, unsigned char * ciphertext, int len);


//Compute the xor of two arrays
void xor_array(int xor_len, unsigned char a[], unsigned char b[], unsigned char res[])
{
	for (int i=0; i < xor_len; i++)
	{
		res[i] = a[i] ^ b[i];
	}
}

//Compute the xor of two elements from G1
void  xor_G1(int size, int xor_len, G1* a, unsigned char b[], unsigned char res[])
{
	// convert a to bytes
	unsigned char a_tmp[size];
	a->toBytes(a_tmp);

	xor_array(xor_len, a_tmp, b, res);
}

//Datatype for the elements returned from phase 2
struct Phase2_return
{
	G1 g_r;
	unsigned char ** ciphertext;
	unsigned char ** iv;
	unsigned char ** enc;
	int message_len;
};

// Compute the hash of a element from G1
void hash_G1(Pairing &e, int len, G1 *arg, G1 *res)
{
	unsigned char a[len];
	arg->toBytes(a);
	*res = G1(e, a, len);
}


// Perform Oblivious transfer between specified sender and recipien
bool ot(OT_Sender* sender, OT_Recipient* recipient, Pairing &e, G1 g, int part)
{

  int len = e.get_length_in_bytes_G1();

  // Initialize parties
  G1 c = sender->init(len, e, g);
  recipient->init(len, e, g, c);

  // Call phases 1-3
  G1 pk0 = recipient->phase1();
  Phase2_return ret = sender->phase2(pk0);
  recipient->phase3(ret);

// Debugging code
/*
  unsigned char ** key  = sender->get_key();
  unsigned char * result = recipient->getResult();
  int sigma = recipient->getSigma();
  for (int i = 0; i < KEYLENGTH; i++){
    if (key[sigma][i] != result[i]){
      cerr << "ERROR in part " << part << ": Position " << i << ": Key: " << key[sigma][i] << ", Res: " << result[i] << endl; 
    }
  }
  if( strcmp( (char *) key[sigma], (char *) result)){
    cerr << "ERROR \n";
    cerr << "original Message  : " << key[sigma]<< endl;
    cerr << "enctypted Message : " << ret.enc[sigma] << endl;
    cerr << "resulting Message : " << result << endl;
  } 
*/
  return true;
}



#endif
