// #include "crypto.h"
#include "cryptopp/osrng.h"
#include <iostream>
#include <string>
#include "crypto++/sha.h"
#include "crypto++/hex.h"
#include <iostream>


using namespace std;
using namespace CryptoPP;

const char * sha256_G1(G1 * arg)
{
	SHA256 hash;
	string g1_str = arg->toString(false);
	cerr << "In sha256() G1 string = " << g1_str << endl;
	string digest;
 
	StringSource s(g1_str, true, new HashFilter(hash, new HexEncoder(new StringSink(digest))));

	return digest.c_str();
}
void xor_G1(int size, int xor_len, G1* a, unsigned char b[], unsigned char res[])
{
	// convert a to bytes
	unsigned char a_tmp[size];
	a->toBytes(a_tmp);

	xor_array(xor_len, a_tmp, b, res);
}
void xor_array(int xor_len, unsigned char a[], unsigned char b[], unsigned char res[])
{
	for (int i=0; i < xor_len; i++)
	{
		res[i] = a[i] ^ b[i];
	}
}
void hash_G1(Pairing &e, int len, G1 *arg, G1 *res)
{
	unsigned char a[len];
	arg->toBytes(a);
	*res = G1(e, a, len);
}


unsigned char* encrypt(unsigned char * key_, unsigned char* iv_, unsigned char * plaintext, int len)
{
	AutoSeededRandomPool prng;

	//unsigned char* key = (unsigned char *) malloc (sizeof(unsigned char) * KEYLENGTH);
	unsigned char* 	key = key_;
	prng.GenerateBlock(key, sizeof(unsigned char) * KEYLENGTH );

	unsigned char* iv = (unsigned char *) malloc (sizeof(unsigned char) * AES::BLOCKSIZE);
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );
	iv_ = iv;
	prng.GenerateBlock(iv, sizeof(unsigned char) * AES::BLOCKSIZE);

	string plain(plaintext, plaintext + len);
	string cipher;


	try
	{
		//cout << "plain text: " << plain << endl;

		CTR_Mode< AES >::Encryption e;
		e.SetKeyWithIV(key, sizeof(unsigned char) * KEYLENGTH, iv);

		// The StreamTransformationFilter adds padding
		//  as required. ECB and CBC Mode must be padded
		//  to the block size of the cipher.
		StringSource(plain, true, 
			new StreamTransformationFilter(e,
				new StringSink(cipher)
			) // StreamTransformationFilter      
		); // StringSource
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	unsigned char * ret = (unsigned char *) malloc (sizeof(unsigned char) * len);
	copy(cipher.begin(), cipher.end(), ret);
	return ret;
}

unsigned char* decrypt(unsigned char* key, unsigned char* iv, unsigned char * ciphertext, int len)
{
	string cipher(ciphertext, ciphertext + len);
	string recovered;

	try
	{
		CTR_Mode< AES >::Decryption d;
		d.SetKeyWithIV(key, sizeof(unsigned char) * KEYLENGTH ,  iv);

		// The StreamTransformationFilter removes
		//  padding as required.
		StringSource s(cipher, true, 
			new StreamTransformationFilter(d,
				new StringSink(recovered)
			) // StreamTransformationFilter
		); // StringSource

		// cout << "recovered text: " << recovered << endl;
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	unsigned char * ret = (unsigned char *) malloc (sizeof(unsigned char) * len);
	copy(recovered.begin(), recovered.end(), ret);

	return ret;
}


unsigned char* encrypt_new(unsigned char * key_, unsigned char* iv_, unsigned char * plaintext, int len)
{
	cerr << "In encrypt_new()" << endl << endl;
	cerr << "default AES keylength : " << AES::DEFAULT_KEYLENGTH << endl;
	cerr << "AES BLOCKSIZE : " << AES::BLOCKSIZE << endl;
	cerr << "Key recieved : " << key_ << endl;

	AutoSeededRandomPool prng;

	//byte key[AES::DEFAULT_KEYLENGTH];
	unsigned char *key = key_;

	prng.GenerateBlock(key, sizeof(key));

	byte iv[AES::BLOCKSIZE];
	prng.GenerateBlock(iv, sizeof(iv));

	string plain = "CTR Mode Test";
	string cipher, encoded, recovered;

	// Pretty print key
	encoded.clear();
	StringSource(key, sizeof(key), true,
		new HexEncoder(
			new StringSink(encoded)
		) 
	);
	cerr << "encoded key: " << encoded << endl;

	// Pretty print iv
	encoded.clear();
	StringSource(iv, sizeof(iv), true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "encoded iv: " << encoded << endl;

	try
	{
		cout << "plain text: " << plain << endl;

		CTR_Mode< AES >::Encryption e;
		e.SetKeyWithIV(key, sizeof(key), iv);

		StringSource(plain, true, 
			new StreamTransformationFilter(e,
				new StringSink(cipher)
			) 
		); 
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	// Pretty print
	encoded.clear();
	StringSource(cipher, true,
		new HexEncoder(
			new StringSink(encoded)
		) 
	); 
	cerr << "cipher text: " << encoded << endl;

	return (unsigned char*) encoded.c_str();

}
