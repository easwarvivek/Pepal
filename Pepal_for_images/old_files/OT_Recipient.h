#ifndef __OT_RECIPIENT_H__
#define __OT_RECIPIENT_H__

#include "crypto.h"

// Class for the recipient in Oblivious Transfer
class OT_Recipient
{
	private:
		int len;
		Pairing *e;
		G1 g;
		int sigma;
		G1 c;
		Zr k;
		G1 pk[2];
		unsigned char * result;
		unsigned char ** result_part;
		int * message_len;
	
	public:
    // Constructor
		OT_Recipient(int sigma, unsigned char ** result_part, int* message_len)
		{
			this->sigma = sigma;
			this->result_part = result_part;
			this->message_len = message_len;
		}

    // Initialization
		void init(int len, Pairing &e, G1 g, G1 c)
		{
			this->len = len;
			this->e = &e;
			this->g = g;
			this->c = c;
		}

    // Code for phase 1
		G1 phase1(){
			k = Zr(*e, true);
			pk[sigma] = g^k;	
			pk[1-sigma] = c*(pk[sigma].inverse());
			return pk[0];
		}

    // Code for phase 2
		void phase3(Phase2_return ret)
		{
			G1 g_r_k = ret.g_r^k;
			G1 h_g_r_k;
			hash_G1(*e, len, &g_r_k, &h_g_r_k);


			result = (unsigned char *) malloc(sizeof(char) * (KEYLENGTH+1));
			result[KEYLENGTH] = '\0';
			xor_G1(len, KEYLENGTH, &h_g_r_k , ret.enc[sigma], result);

			*message_len = ret.message_len;

			*result_part = decrypt(result, ret.iv[sigma], ret.ciphertext[sigma], ret.message_len);
		}
	
		unsigned char* getResult()
		{
			return result;
		}	
		int getSigma()
		{
			return sigma;
		}
};
#endif