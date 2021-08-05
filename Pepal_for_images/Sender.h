#ifndef __SENDER_H__
#define __SENDER_H__

#include "crypto.h"


// test comment
class OT_Sender
{
	private:		
		unsigned char **ciphertext; // encryptedMessage -> [char *e0, char *e1], e0.length = message_len
		unsigned char **message; // message -> [char *m0, char *m1]
		unsigned char **iv; // initialVector -> [char *v0, char *v1]   (for encryption security)
		unsigned char **key; // SHA256(k[i]) -> [char *k0, char *k1]
		G1 A;
		int message_len; // length of m0 and m1
		encrypted_return ret;
		Zr a;

	public:
     
		OT_Sender(unsigned char **message, int message_len, G1 Sender_A,  Zr a)
		{
			this->message  = message;
			this->message_len = message_len;
			this->A = Sender_A;
			this->a = a;
			
			ciphertext = new unsigned char * [2];
			iv = new unsigned char * [2];			
			key = new unsigned char * [2];	
		}				
    	
		void getEncrypted(G1 B, encrypted_return * ret) 
		{
			if(DEBUG)
				cerr << "***In getEncrypted()" << endl << endl;
			
			// create k0 & k1
			G1 k[2]; 
			k[0] = B^a;
			k[1] = (B*(A.inverse()))^a;			
			
			// send message length to decryption
			ret->message_len = this->message_len;
			if(DEBUG)
				cerr << "Creating ciphers with len = " << this->message_len << endl; 

			for (int i = 0; i <  2; i++) // key init & encrypt
			{
				string hashedKey = sha256_G1( &k[i] );
				key[i] =  (unsigned char *) malloc (sizeof(unsigned char) * (hashedKey.size() + 1) ); 
				copy( hashedKey.begin(), hashedKey.end(), key[i] );

				string cipher_str = encrypt_new(key[i], iv[i], message[i], this->message_len);
				ciphertext[i] = (unsigned char *) malloc (sizeof(unsigned char) * (cipher_str.size() + 1) ); 
				copy( cipher_str.begin(), cipher_str.end(), ciphertext[i] );
				
				ret->ciphertext[i] = cipher_str;				
				ret->message_len = cipher_str.length();

				if(DEBUG)
					cerr << "HashedKey[" << i << "] :" <<  key[i] << endl;				
			}
			if(DEBUG)
				cerr << "***Leaving getEncrypted() " << endl;			
		}


		unsigned char ** get_message()
		{
			return message; // pointer to [char * m0, char * m1]
		}
		unsigned char ** get_cipher() // should be called after getEncrypted()
		{
			return ciphertext;
		}
		G1 getA()
		{
			return this->A;
		}
};

// Class for sender 
class Sender
{
	private:
		char * input_file_name;
		int sec_par; 
		int message_len;
		unsigned char *** message;
		Pairing *e;
		G1 g;
		Zr a;
		G1 A;				

	public:

		Sender(char *input_file_name, int sec_par, Pairing &e)
		{
			this->sec_par = sec_par;
			// Create message parts
			message = new unsigned char ** [sec_par];
		}

		G1 product_Bs(Pairing &e, std::vector<G1> B_arr )
		{
			G1 product_Bs = G1(e, true); // set identity element to true
			for(int i = 0; i < sec_par; i++)
			{
				Zr pw = Zr(e, (long int)2);
				Zr temp = Zr(e, (long int)i);
				pw = pw^temp;
				G1 toPower = (B_arr[i])^pw; // pw = 2^i
				product_Bs *= toPower;
			}			
			return product_Bs;
		}
		Zr get_rb(Pairing &e)
		{
			Zr res = Zr(e, true);
			return res;
		}

		int fill_msg() // temp fill the message array with { [mo, m1] , ...} 
		{
			int m_len = 1000;

			for (int i = 0; i < sec_par; i++)
			{
				message[i] = new unsigned char * [2];
				for (int j = 0; j < 2; j++)
				{					
					message [i][j] = new unsigned char[m_len];
					for(int k = 0; k < m_len; k++)
					{
						if(j == 0)
							message[i][j][k] = '0';
						else
							message[i][j][k] = '1';
					}					
				}
			}
			this->message_len = m_len;
			return m_len;
		}			

    	// Get new OT_Sender for a specific message part
		OT_Sender* new_OT_Sender(int part)
		{
			OT_Sender* ret = new OT_Sender(message[part], message_len, A, a) ;
			return ret;
		} 

		G1 initA(Pairing &e, G1 g)
		{
			this->e = &e;
			this->g = g;
			// raise g to a random power
			this->a = Zr(*this->e, true);
			A = g^a;
			this->A = A;	
			return A;
		}
		Zr get_a()
		{
			return this->a;
		}

		int watermark(char *input_file_name) // watermarks image and reads it into a char[][] message
		{
			this->input_file_name = input_file_name;      		

      		// Code for watermarking
			char str[200] ;
			char wm_algo[20] = WM_ALGO;
			char gen_param_large[20] = "";
			char gen_param_small[20] = "-a 0.1";
			char input1[50] = "";
			char input2[50] = "";
			char input3[50] = "";
			//char newWmDir[50] = "new_wm_src";
      		// Some watermarks contain a message
			if (NEED_INPUT)
			{
				sprintf(input1,"echo \"I am responsible for this file\" | ");
				sprintf(input2,"echo \"0\" | ");
				sprintf(input3,"echo \"1\" | ");
			}

      		// Call external watermarking code

      		int systemErr = 1;
      		if(DEBUG)
				cerr << "begin watermarking with sec_par = " << sec_par << endl << endl;

      		
      		//Watermark on entire file
			sprintf(str," %swatermarking/new_wm_src/gen_%s_sig %s -o /tmp/NDA/%s.sig", input1, wm_algo, gen_param_large, wm_algo);
			systemErr =system(str);
			sprintf(str, "watermarking/new_wm_src/wm_%s_e -s /tmp/NDA/%s.sig -o /tmp/NDA/%s_out.pgm %s", wm_algo, wm_algo, wm_algo, input_file_name);
			systemErr =system(str);
			

      		//split watermarked file into single parts
			sprintf(str,"pgm_join_split/split /tmp/NDA/%s_out.pgm 16 16", wm_algo);
			if(DEBUG || 1)
				cerr << "splitting " << input_file_name << "  complete. Output files in /tmp/NDA/cox_out_[0, 1]_[0-256].pgm" << endl;
			systemErr = system(str);
			
      		//Watermark on single parts
			sprintf(str, "%s watermarking/new_wm_src/gen_%s_sig %s -o /tmp/NDA/%s_0.sig", input2, wm_algo, gen_param_small, wm_algo);
			systemErr =system(str);
			sprintf(str, "%s watermarking/new_wm_src/gen_%s_sig %s -o /tmp/NDA/%s_1.sig", input3, wm_algo, gen_param_small, wm_algo);
			systemErr =system(str);
			
			for (int i = 0; i< sec_par; i++)
			{
				message[i] = new unsigned char * [2];
				for (int j = 0; j<2; j++)
				{
					sprintf(str, "watermarking/new_wm_src/wm_%s_e -s /tmp/NDA/%s_%d.sig -o /tmp/NDA/%s_out%d_%d.pgm /tmp/NDA/out%d.pgm",wm_algo,wm_algo,j,wm_algo,j,i,i); 
					//cout << str << endl;
					systemErr =system(str);

          			// Read watermarked parts to array
					sprintf(str, "/tmp/NDA/%s_out%d_%d.pgm",wm_algo,j,i);
					ifstream fl(str);  
					fl.seekg( 0, ios::end );  
					size_t len = fl.tellg();  
					message [i][j] = new unsigned char[len];  
					fl.seekg(0, ios::beg);   
					fl.read((char *) (message [i][j]), len);  
					fl.close();  
					message_len = len;
				}
			}
			if (systemErr  < 0)
				cerr << "system() call failed in watermarking()" << endl;
			return message_len;
		}	
};


#endif
