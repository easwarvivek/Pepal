#ifndef __SENDER_H__
#define __SENDER_H__

#include "crypto.h"

using namespace std;

// Class for sender 
class Sender
{
	private:
		char * input_file_name;
		int sec_par;
		int message_len;
		unsigned char *** message;
		unsigned char *** key;
		unsigned char *** ciphertext;
		unsigned char *** iv;
	    G1 g;
	    Zr sk;
	    G1 vk;

	public:
    	// Constructor, also does the first protocol steps
		Sender(char * input_file_name, int sec_par, Pairing& e)
		{
			this->input_file_name = input_file_name;
			this->sec_par = sec_par;
      		// Create message parts
			message = new unsigned char ** [sec_par];
      		// Code for 
			timeval tstart, tfinish, tstart2, tfinish2, tstart3, tfinish3;
			double duration;
			gettimeofday(&tstart, NULL);
      		// Code for watermarking
			char str[200] ;
			char wm_algo[20] = WM_ALGO;
			char gen_param_large[20] = "";
			char gen_param_small[20] = "-a 0.1";
			char input1[50] = "";
			char input2[50] = "";
			char input3[50] = "";
      		// Some watermarks contain a message
			if (NEED_INPUT)
			{
				sprintf(input1,"echo \"I am responsible for this file\" | ");
				sprintf(input2,"echo \"0\" | ");
				sprintf(input3,"echo \"1\" | ");
			}
      		// Call external watermarking code
      		
      		//Watermark on entire file
      		/*
			sprintf(str," %swatermarking/Meerwald/gen_%s_sig %s -o /tmp/LIME/%s.sig", input1, wm_algo, gen_param_large, wm_algo);
			system(str);
			sprintf(str, "watermarking/Meerwald/wm_%s_e -s /tmp/LIME/%s.sig -o /tmp/LIME/%s_out.pgm %s", wm_algo, wm_algo, wm_algo, input_file_name);
			system(str);

      		//split watermarked file into single parts
			sprintf(str,"pgm_join_split/split /tmp/LIME/%s_out.pgm %d", wm_algo, sec_par);
			system(str);
	
      		//Watermark on single parts
			sprintf(str, "%s watermarking/Meerwald/gen_%s_sig %s -o /tmp/LIME/%s_0.sig", input2, wm_algo, gen_param_small, wm_algo);
			system(str);
			sprintf(str, "%s watermarking/Meerwald/gen_%s_sig %s -o /tmp/LIME/%s_1.sig", input3, wm_algo, gen_param_small, wm_algo);
			system(str);

			for (int i = 0; i< sec_par; i++)
			{
				message[i] = new unsigned char * [2];
				for (int j = 0; j<2; j++)
				{
					sprintf(str, "watermarking/Meerwald/wm_%s_e -s /tmp/LIME/%s_%d.sig -o /tmp/LIME/%s_out%d_%d.pgm /tmp/LIME/out%d.pgm",wm_algo,wm_algo,j,wm_algo,j,i,i); 
					cout << str << endl;
					system(str);

          			// Read watermarked parts to array
					sprintf(str, "/tmp/LIME/%s_out%d_%d.pgm",wm_algo,j,i);
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
			gettimeofday(&tfinish, NULL);
			duration = (tfinish.tv_sec - tstart.tv_sec) * 1000.0 ;
			duration += (tfinish.tv_usec - tstart.tv_usec) / 1000.0 ;
			cout << "watermarking: " << duration << endl;
			


      		// Computation of Signatures (BLS scheme)
			gettimeofday(&tstart2, NULL);

			int len_of_group_elem_in_bytes = e.get_length_in_bytes_G1();
			g  = G1(e,true);
			sk = Zr(e,false); 
			vk = g ^ sk;

			char *** sig_mes;
			unsigned char *** sig;
			sig_mes = new char ** [sec_par];
			sig = new unsigned char ** [sec_par];

      		// Do this for each (watermarked) part of the message
			for (int i = 0; i< sec_par; i++)
			{
		        sig_mes [i] = new char* [2];
		        sig [i] = new unsigned char* [2];
				for (int j = 0; j<2; j++)
				{
		          sig_mes [i][j] = new char [25];
		          sig [i][j] = new unsigned char [len_of_group_elem_in_bytes];
		          sprintf(sig_mes[i][j], "%ld,%d,%d", (long)time(NULL),i,j);
		          G1 hash = G1(e, sig_mes, 25); 
		          G1 signature = hash ^ sk;
		          signature.toBytes(sig[i][j]);
				}
			}
			gettimeofday(&tfinish2, NULL);
			duration = (tfinish2.tv_sec - tstart2.tv_sec) * 1000.0 ;
			duration += (tfinish2.tv_usec - tstart2.tv_usec) / 1000.0 ;
			cout << "signature: " << duration << endl;

      		//Encryptions
			gettimeofday(&tstart3, NULL);

			key = new unsigned char ** [sec_par];
			ciphertext = new unsigned char ** [sec_par];
			iv = new unsigned char ** [sec_par];

      		// Encrypt all (signed + watermarked) parts of the message
			for (int i = 0; i< sec_par; i++)
			{
				key[i] = new unsigned char * [2];
				ciphertext[i] = new unsigned char * [2];
				iv[i] = new unsigned char * [2];
				for (int j = 0; j<2; j++)
				{
					unsigned char * raw_message = new unsigned char [message_len + len_of_group_elem_in_bytes + 1];
					memcpy(raw_message, message[i][j],message_len);
					memcpy(raw_message + message_len, sig_mes[i][j], len_of_group_elem_in_bytes);

					ciphertext[i][j] = encrypt(&key[i][j], &iv[i][j], raw_message, message_len + len_of_group_elem_in_bytes);
				}
			}

			gettimeofday(&tfinish3, NULL);
			duration = (tfinish3.tv_sec - tstart3.tv_sec) * 1000.0 ;
			duration += (tfinish3.tv_usec - tstart3.tv_usec) / 1000.0 ;

			cout << "encryption: " << duration << endl;
			*/
		}	

    	// Get new OT_Sender for a specific message part
		OT_Sender* new_OT_Sender(int part)
		{
			OT_Sender* ret = new OT_Sender(key[part], ciphertext[part], iv[part],message_len);
			return ret;
		}
};
#endif