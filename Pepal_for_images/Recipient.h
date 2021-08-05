#ifndef __RECIPIENT_H__
#define __RECIPIENT_H__

#include "crypto.h"

using namespace std;

// Class for the recipient in Oblivious Transfer
class OT_Recipient
{
	private:
		int len;
		Pairing *e;
		G1 B;		
		Zr b;
		int bit_c;
		unsigned char **result_part;
		unsigned char **key;
		int message_len;
	
	public:
    	OT_Recipient(unsigned char ** result_part, int message_len, int c, G1 B, Zr b)
		{
			this->bit_c = c;
			this->B = B;
			this->b = b;
			this->message_len = message_len;
			this->result_part = result_part;
			key = new unsigned char * [2];
		}

		void makeKeys(G1 A)
		{
			if(DEBUG)
				cerr << "***In Recipient makeKeys() " << endl;

			G1 k[2];

			for (int i = 0; i <  2; i++)
			{
				k[i] = A^b;
				string hashedKey = sha256_G1( &k[i] );
				key[i] = (unsigned char *) malloc (sizeof(unsigned char) * (hashedKey.size() + 1) ); 
				copy( hashedKey.begin(), hashedKey.end(), key[i] );

				if(DEBUG)
					cerr << "Recipient HashedKey[" << i << "] :" <<  key[i] << endl;
			}				
		} 

		void decryptE( encrypted_return *ert)
		{
			if(DEBUG)
			{
				cerr << "***In decryptE() " << endl << endl;			
				cerr << "cipher_len : " << ert->message_len << endl;
				cerr << "Cipher to decrypt : " << ert->ciphertext[bit_c] << endl;
			}
			
			int c_len = ert->message_len;
			if(bit_c == 0)
				*result_part = decrypt_new(key[0], ert->iv[0], ert->ciphertext[0], c_len);				
			else
				*result_part = decrypt_new(key[1], ert->iv[1], ert->ciphertext[1], c_len);	

			if(DEBUG)
				cerr << "***Leaving decryptE() " << endl << endl;
		}

		G1 getB()
		{
			return B;
		}  
};


// Class for Recipient
class Recipient
{
	private:
		vector<int> bitstring;
		int sec_par;
		unsigned char ** result;
		int message_len;		
		Pairing *e;
		vector<G1> B_arr;
		vector<Zr> b_arr;
		Zr sk;
		Zr x;
		G1 u;
		Zr r;

	public:
    
		Recipient(Zr sk, vector<int> sk_bitstring, int sec_par, int message_len)
		{
			this->bitstring = sk_bitstring;
			this->sec_par = sec_par;
			this->sk = sk;
			this->message_len = message_len;
			result = new unsigned char * [sec_par];				
		}
		Zr init_x(Pairing &e, G1 g)
		{
			Zr power = Zr(e, (long int) 0);
			for(int k = 0; k < sec_par; k++)
			{
				Zr two = Zr(e, (long int)2);
				Zr i = Zr(e, (long int)k);
				power += (two^i) * b_arr[k]; 
			}
			this->x = power;
			return power;
		}
		Zr get_z(Pairing &e, G1 g, Zr b)
		{
			// return z = r + bx
			return this->r + ((this->x)*b);
		}
		Zr get_x()
		{
			return this->x;
		}
		G1 set_u(Pairing &e, G1 g)
		{
			Zr r = Zr(e, true);
			this->r = r;
			this->u = g^r;
			return this->u;
		}

		G1 get_Cb( G1 g)
		{
			
			G1 res = g^(this->x);
			// cerr << "g^(SUM( (2^i)*b_i ): ";
			// printHex_G1(res);
			return res;
		}
		G1 get_Cs(Pairing &e, G1 g)
		{
			G1 res = g^sk;			
			// cerr << "g^(sk): ";
			// printHex_G1(res);
			return res;
		}

		void initB_arr(Pairing &e, G1 A, G1 public_g)
		{
			if(DEBUG)
				cerr << "In initB_arr() with sec_par: " << sec_par << endl;

			for(int i = 0; i < this->sec_par; i++)
			{				
				Zr new_bi = Zr(e, true);
				b_arr.push_back(new_bi); 

				G1 new_Bi;
				if( !bitstring[sec_par-1 - i] ) // bitstring is msb to lsb so reverse order
					new_Bi = public_g^(new_bi);
				else
					new_Bi = A* ( public_g^(new_bi) );
				B_arr.push_back(new_Bi);				
			}
			if(DEBUG)
				cerr << "B_arr generated with size():" << B_arr.size() << endl;				
		}

		OT_Recipient* new_OT_Recipient(int part)
		{
			//
			// NOTE: bitstring is (msb to lsb) and Bi's are generated in reverse order.
			//
			int sp_p = (sec_par -1 ) - part; // sec_par to part
			//sp_p = part;
			OT_Recipient* ret = new OT_Recipient(&result[part], message_len, bitstring[sp_p], B_arr[part], b_arr[part] );

			if(DEBUG)
				cerr << "New OT recipient for bit_C = " << bitstring[part] << endl;
			return ret;
		}

		std::vector<G1> getB_arr()
		{
			return B_arr;
		}

		std::vector<Zr> getb_arr()
		{
			return b_arr;
		}		

		void temp_finish()
		{
			cerr << "---OT complete for all file sections---" << endl << endl; 
			cerr << "### Recipient(signature bits = ";
			for (int i = 0; i < sec_par; i++)
        		cerr << bitstring[i];
    		cerr << ")" << endl << "---Complete message recieved after OT---" << endl << endl;

			for (int i = 0; i < sec_par; i++)
			{
				for (int j = 0; j < message_len; j++)
					cout << result[i][j];				
			}      		
		}

		void finish()
		{
			int systemErr = 1;
			char str[128];
      		// Dump the message parts as pgm files 
			for (int i = 0; i < sec_par; i++)
			{
				sprintf(str, "/tmp/NDA/rec_out%d.pgm", i);
				ofstream out(str); 

				for (int j = 0; j < message_len; j++)
					out << result[i][j];
				
				out.close();
			}

      		// Join all single parts to obtain the final image
			sprintf(str,"pgm_join_split/join /tmp/NDA/rec_out %d", sec_par);
			systemErr = system(str);

			if(systemErr < 0)
				cerr << "system() call failed in recipient finish" << endl;
		}
};

#endif
