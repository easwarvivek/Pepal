#ifndef __OT_SENDER_H__
#define __OT_SENDER_H__

#include "crypto.h"

class OT_Sender
{
	// class for the sender in Oblivious transfer
	private:
		int len;
		Pairing *e;
		G1 g;
		unsigned char** ciphertext;
		unsigned char** key;
		unsigned char** iv;
		G1 c;
		int message_len;

	public:
    // Constructor 
		OT_Sender(unsigned char** key, unsigned char ** ciphertext, 
			unsigned char ** iv, int message_len)
		{
			this->key  = key;
			this->ciphertext  = ciphertext;
			this->iv  = iv;
			this->message_len = message_len;
		}

    // Initialization
		G1 init(int len, Pairing &e, G1 g) 
		{
			this->len = len;
			this->e = &e;
			this->g = g;

			c= G1(*this->e, false);
			return c;
		}
		
    // The code for phase 2
		Phase2_return phase2(G1 pk0)
		{
			// Recompute PK_1
			G1 pk[2]; 
			pk[0] = pk0;
			pk[1] = c*(pk[0].inverse());
	
			// random r
			Zr r = Zr(*e,true);

			Phase2_return ret;
			ret.enc  = (unsigned char **) malloc(sizeof(char) * (2));
			for (int i = 0; i < 2; i ++){
				ret.enc[i] = (unsigned char *) malloc (sizeof(char) * (len+1));
			}

			ret.g_r = g^r;
			ret.ciphertext=ciphertext;
			ret.iv=iv;
			ret.message_len = message_len;

			// Encryption
			G1 pk_r[2];
			G1 h_pk_r[2];

			for (int i = 0; i <2; i++)
			{
				ret.enc[i][len] = '\0';
				// Exponentiation
				pk_r[i] = pk[i]^r;
				// Hashing
				hash_G1(*e, len, &pk_r[i], &h_pk_r[i]);
				// Encryption
				xor_G1(len, KEYLENGTH, &h_pk_r[i],key[i],ret.enc[i]);
			}
			return ret;
		}
		unsigned char ** get_key()
		{
			return key;
		}		
};
#endif