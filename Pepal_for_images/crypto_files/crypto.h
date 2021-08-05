#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/filters.h"
#include "cryptopp/ccm.h"
#include "cryptopp/aes.h"
#include "cryptopp/sha.h"
#include "cryptopp/hex.h"

#include "PBC.h"
#include "assert.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <vector>
#include <math.h>
#include <sstream>
#include <bitset>

#include "memstream/memstream.h"

using namespace std;
using namespace CryptoPP;
using CryptoPP::CTR_Mode;
using CryptoPP::AES;
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;
using CryptoPP::Exception;
using CryptoPP::AutoSeededRandomPool;


#define KEYLENGTH 32 // 32 bytes = 256  bits for the SHA256
#define WM_ALGO "cox"
#define NEED_INPUT 0
#define BLIND 0

#define DEBUG 0



struct encrypted_return
{
	int message_len;

	string ciphertext[2];
	unsigned char ** iv;
};

string sha256_G1(G1 * arg)
{	
	string g1_str = arg->toString(true);

	// if(DEBUG)
	// 	cerr << "In sha256() G1 string = " << g1_str << endl;

	SHA256 hash;
	string digest; 
	StringSource s(g1_str, true, new HashFilter(hash, new HexEncoder(new StringSink(digest))));

	// if(DEBUG)
	// {
	// 	cerr << "sha256(G1) = " << digest << endl;
	// 	cerr << "length of hash = "<< digest.size() << endl;
	// }
	return digest;
}
void xor_array(int xor_len, unsigned char a[], unsigned char b[], unsigned char res[])
{
	for (int i=0; i < xor_len; i++)
	{
		res[i] = a[i] ^ b[i];
	}
}
string encrypt_new(unsigned char * key_, unsigned char* iv_, unsigned char * plaintext, int m_len)
{
	if(DEBUG)
	{
		cerr << endl << "***In encrypt_new()***" << endl << endl;
		cerr << "NDA KEYLENGTH & BLOCKSIZE : " << KEYLENGTH << endl;
	}

	unsigned char *key = key_;		
	string plain( plaintext, plaintext + m_len );	

	// init & print iv 
	unsigned char* iv = (unsigned char *) calloc (KEYLENGTH*sizeof(unsigned char) , sizeof(unsigned char) );	
	iv_ = iv;	
	string encoded;	
	encoded.clear();
	StringSource(iv_, KEYLENGTH, true, new HexEncoder(new StringSink(encoded)) );

	if(DEBUG)
	{
		cerr << "Hex encoded iv: " << encoded << endl;	
		cerr << "Encrypting with Key :" << key << endl;
	}

	try
	{
		if(DEBUG)
			cerr << "plain text to encrypt: " << plain << endl;

		string cipher;
		CTR_Mode< AES >::Encryption e;
		e.SetKeyWithIV(key, KEYLENGTH*sizeof(unsigned char), iv_);
		StringSource(plain, true, new StreamTransformationFilter(e, new StringSink(cipher)) ); 

		if(DEBUG)
		{
			// print cipher
			cerr << "Encrypted cipher text: " << cipher << endl;
			cerr << "***Leaving encrypt_new()***" << endl << endl;
		}
		
		return cipher;		
	}
	catch(const CryptoPP::Exception& e)
	{
		if(DEBUG)
			cerr << "Encrytion failed in encrypt_new()" << endl;
		
		if(DEBUG)
			cerr << e.what() << endl;
		exit(1);
	}
	
}
unsigned char* decrypt_new(unsigned char* key, unsigned char* iv, string ciphertext, int len)
{
	if(DEBUG)
		cerr << "***In decrypt_new()" << endl;

	string recovered;

	// init & print iv 
	string encoded;
	unsigned char* iv_ = (unsigned char *) calloc (KEYLENGTH*sizeof(unsigned char), sizeof(unsigned char) );
	encoded.clear();
	StringSource(iv_, KEYLENGTH, true, new HexEncoder(new StringSink(encoded)) );

	if(DEBUG)
		cerr << "Hex encoded iv: " << encoded << endl;

	try
	{
		CTR_Mode< AES >::Decryption d;
		d.SetKeyWithIV(key, sizeof(unsigned char) * KEYLENGTH ,  iv_);
		StringSource s(ciphertext, true, new StreamTransformationFilter(d, new StringSink(recovered)) );

		if(DEBUG)
			cerr << "decrypted text: " << recovered << endl;
	}
	catch(const CryptoPP::Exception& e)
	{
		if(DEBUG)
			cerr << "Decryption failed in decrypt() "<< endl;
		if(DEBUG)
			cerr << e.what() << endl;
		exit(1);
	}

	unsigned char * ret = (unsigned char *) malloc (sizeof(unsigned char) * len);
	copy(recovered.begin(), recovered.end(), ret);

	return ret;
}
vector<int> Zr_to_bs(Pairing &e, Zr sk)
{
	// Generate random watermarking bitstring -> replace with Bitcoin signature
  	vector<int> res;
  	
  	int len = sk.getElementSize();
  	string sk_str = sk.toString();

  	string encoded;	
	encoded.clear();
	StringSource( (const unsigned char *) sk_str.c_str(), len-1, true, new HexEncoder(new StringSink(encoded)) );
	if(DEBUG)
			cerr << "sk[HEX] : " << encoded << endl;

	char *sk_bits;
  	size_t size;
  	FILE *stream;
  	stream = open_memstream (&sk_bits, &size);

  	//cerr << "Dumping sk to stdout with dump()" << endl;
  	//sk.dump(stdout, NULL, 2);
  	if(DEBUG)
			cerr << "Dumping sk bits to string with dump()" << endl;
  	sk.dump(stream, NULL, 2);
  	fflush (stream);
  	fclose (stream);
  	if(DEBUG)
  		printf ("Dumped_str = %s, sk_bitSize = %d\n", sk_bits, (int)size-1);

  	if(DEBUG)
			cerr << "Created vector<int> : ";

	for (int i = 0; i < (int)size-1; i++)
	{
		if(sk_bits[i] == '0')
			res.push_back(0);
		else if(sk_bits[i] == '1')
			res.push_back(1);
		// cerr << "res_i : " << res[i] << " sk_bits_i : " << sk_bits[i] << endl;
		if(DEBUG)
			cerr << res[i];
	}
	if(DEBUG)
			cerr << endl <<"vector size: " << res.size() << endl;

	return res;
}

void printHex_G1(G1 toPrint)
{
	G1 res = toPrint;
	string ts = res.toString(false);

	string encoded;	
	encoded.clear();
	StringSource( (const unsigned char *)ts.c_str() , ts.size(), true, 
				new HexEncoder(new StringSink(encoded)) );
	if(DEBUG)
			cerr << encoded << endl;
}

#endif
