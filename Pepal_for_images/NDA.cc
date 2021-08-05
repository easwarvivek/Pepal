#include "crypto.h"
#include "PBC.h"
#include "sys/time.h"
#include "crypto.h"
#include "Recipient.h"
#include "Sender.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

using namespace std;

double s_ot_time =0, r_ot_time=0;
clock_t transfer_s, transfer_e;


// Perform Oblivious transfer between sender and recipient 
// ot(ot_s, ot_r, e, public_g, i);
bool ot(OT_Sender* ot_s, OT_Recipient* ot_r, Pairing &e, G1 public_g, int part)
{
	if(DEBUG)
		cerr << "-----OT transfer for file section : " << part << " -----" << endl << endl;
	encrypted_return enc;

	transfer_s = clock();
	G1 B = ot_r->getB();
	transfer_e = clock();
	r_ot_time += (transfer_e - transfer_s) / (double)CLOCKS_PER_SEC;

	transfer_s = clock();
	G1 A = ot_s->getA();
	ot_s->getEncrypted(B, &enc);
	transfer_e = clock();
	s_ot_time += (transfer_e - transfer_s) / (double)CLOCKS_PER_SEC;

	transfer_s = clock();
	ot_r->makeKeys(A);
	ot_r->decryptE(&enc);
	transfer_e = clock();
	r_ot_time += (transfer_e - transfer_s) / (double)CLOCKS_PER_SEC;
	return true;
}
bool detect_wm(std::vector<int> bitstring, char *inputfile)
{
	FILE *fp;
	char str[256];
	char pout[256];
	char pout2[256];
	int errorcount = 0;
	int sys_err;
	char* file_read_err;
	int sec_par = bitstring.size();

	// Check if all watermarks for the single parts could be detected correctly
	for (int i = 0; i < sec_par; i++)
	{
	    // Blind watermarking schemes do not need the orignal watermark for verification
	    if (BLIND)
	    {
	      sprintf(str, "watermarking/new_wm_src/wm_%s_d -v 0 -s /tmp/NDA/%s_0.sig -o /tmp/NDA/%s_%d_0.wm /tmp/NDA/rec_out%d.pgm", WM_ALGO, WM_ALGO, WM_ALGO, i,i);
	      sys_err = system(str);
	      sprintf(str, "watermarking/new_wm_src/wm_%s_d -v 0 -s /tmp/NDA/%s_0.sig -o /tmp/NDA/%s_%d_1.wm /tmp/NDA/rec_out%d.pgm", WM_ALGO, WM_ALGO, WM_ALGO, i,i);
	      sys_err = system(str);
	    }
	    else
	    {
	      sprintf(str, "watermarking/new_wm_src/wm_%s_d -v 0 -i /tmp/NDA/out%d.pgm -s /tmp/NDA/%s_0.sig -o /tmp/NDA/%s_%d_0.wm /tmp/NDA/rec_out%d.pgm", WM_ALGO, i,  WM_ALGO, WM_ALGO, i,i);
	      sys_err = system(str);
	      sprintf(str, "watermarking/new_wm_src/wm_%s_d -v 0 -i /tmp/NDA/out%d.pgm -s /tmp/NDA/%s_0.sig -o /tmp/NDA/%s_%d_1.wm /tmp/NDA/rec_out%d.pgm", WM_ALGO, i,  WM_ALGO, WM_ALGO, i,i);
	      sys_err = system(str);
	    }
	

	    sprintf(str, "watermarking/new_wm_src/cmp_%s_sig -C -v 0 -s /tmp/NDA/%s_0.sig /tmp/NDA/%s_%d_0.wm", WM_ALGO,WM_ALGO,WM_ALGO, i);
	    fp = popen(str, "r");
	    file_read_err = fgets(pout, sizeof(pout) - 1, fp);
	    double corr0 = atof(pout);
	    pclose(fp);

	    sprintf(str, "watermarking/new_wm_src/cmp_%s_sig -C -v 0 -s /tmp/NDA/%s_1.sig /tmp/NDA/%s_%d_1.wm", WM_ALGO,WM_ALGO,WM_ALGO, i);
	    fp = popen(str, "r");
	    file_read_err = fgets(pout2, sizeof(pout2) - 1, fp);
	    double corr1 = atof(pout2);
	    pclose(fp);

	    if (( corr0 < corr1) != bitstring[(sec_par-1)-i])
	    {
	      // Count all wrongly detected bits
	      errorcount++;	
	    }
	    //if(file_read_err < 0)
	    if(file_read_err == NULL)
	    	cerr << "file read error in detect_wm" << endl;
	}	

    // Verify that the overall watermark can be detected correctly
    if (BLIND)
    {
    	sprintf(str, "watermarking/new_wm_src/wm_%s_d -s /tmp/NDA/%s.sig -o /tmp/NDA/%s.wm NDA_out_complete.pgm", WM_ALGO, WM_ALGO, WM_ALGO);
    }
    else
    {
    	sprintf(str, "watermarking/new_wm_src/wm_%s_d -i %s -s /tmp/NDA/%s.sig -o /tmp/NDA/%s.wm NDA_out_complete.pgm", WM_ALGO, inputfile, WM_ALGO, WM_ALGO);
    }
    sys_err = system(str);
    sprintf(str, "watermarking/new_wm_src/cmp_%s_sig -s /tmp/NDA/%s.sig /tmp/NDA/%s.wm", WM_ALGO,WM_ALGO,WM_ALGO);
    sys_err = system(str);

    if(sys_err < 0)
    	cerr << "system() call failed in detect_wm" << endl;

    if (errorcount)
    {
    	cout << endl << errorcount << " bits could not be detected correctly\n";
    	return false;
    }    	
    else
	    cout << endl << "All bits were detected correctly!\n";
	return true;
}


int main(int argc, char **argv)
{
	cout << endl;
	int sysErr = 1;

	if (argc != 4)
	{
	  cerr << "Usage: " << argv[0] << " \"image.pgm\" \"sec_par\" \"rand_seed\" \n";
	  return 1;
	}
	
    struct stat st = {0};
    if (stat("/tmp/NDA", &st) == -1) 
        mkdir("/tmp/NDA", 0700);
	
	const char *paramFileName = "pairing.param";
	FILE *paramFile = fopen("pairing.param", "w+");
	Pairing e1;
	e1.generateTypeAPairingParam(paramFile, 256, 1024);
	fclose(paramFile);

	FILE *sysParamFile = fopen(paramFileName, "r");
	if (sysParamFile == NULL) 
	{
		cerr << "Can't open the parameter file " << paramFileName << "\n";
		cerr << "Usage: " << argv[0] << " [paramfile]\n";
		return 0;
	}

	int rand_seed = atoi(argv[3]);
	srand( rand_seed);
	Pairing e(sysParamFile);
	fclose(sysParamFile); 

	int sec_par; // = atoi(argv[2]);
 	char *inputfile = argv[1];


	clock_t start, stop;
	double wm_time, totalTime;

	
 	G1 public_g = G1(e,false);

 	// generate recipient's signing key
 	// Zr R_sk = Zr(e, (long int) 52364);
 	Zr R_sk = Zr(e, true);
 	vector<int> bitstring = Zr_to_bs(e, R_sk);
 	while(bitstring.size() < 256)
 	{
 		//pad the bitstring to make it 256 for 256 rounds of OT
 		bitstring.insert(bitstring.begin(), 0); 
 	}
 	sec_par = bitstring.size();
 	
 	Sender s(inputfile, sec_par, e);
 	start = clock();
 	int message_len = s.watermark(inputfile);
 	stop = clock();
	wm_time = (stop - start) / (double)CLOCKS_PER_SEC;
 	
 	Recipient r(R_sk, bitstring, sec_par, message_len); 


	if(DEBUG)
		cerr << "SK verification..." << endl;
		
	transfer_s = clock();
	start = clock();
	G1 A = s.initA(e, public_g);
	stop = clock();
	s_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;

	start = clock();
 	r.initB_arr(e, A, public_g);
 	Zr x = r.init_x(e, public_g);	
 	G1 Cb = r.get_Cb(public_g);
 	if(DEBUG)
 	{
 		cerr << "[+] g^(SUM( (2^i)*b_i ): ";
		printHex_G1(Cb);
 	}
 	//G1 u = r.set_u(e, public_g);
 	stop = clock();
 	r_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;

 	start = clock();
 	Zr b = s.get_rb(e); // gets random int from sender
 	stop = clock();
 	s_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;

 	// start = clock();
 	// Zr z = r.get_z(e, public_g, b);
 	// stop = clock();
 	// r_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;
 	// if(DEBUG)
 	// {
 	// 	cerr << "[+] g^(r+bx) :";
 	// 	printHex_G1(public_g^z);
 	// 	cerr << "[+] u * Cb^(b) : ";
 	// 	printHex_G1( u * (Cb^b) );
 	// } 		
 	start = clock();
	// if( !( (public_g^z) == ( u * (Cb^b) ) ) )
	// {
	// 	if(DEBUG)
	// 		cerr << "[-] SK verification phase 1a - invalid" << endl;
	// 	exit(-1);
	// }
 	G1 pBs = s.product_Bs(e, r.getB_arr() );
 	stop = clock();
 	s_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;

 	if(DEBUG)
 	{
 		cerr << "[+] PRODUCT( B[i]^(2^i) ): ";
		printHex_G1(pBs); 
 	}				
 	start = clock();
 	G1 Cs = r.get_Cs(e, public_g);
 	stop = clock();
 	r_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;
 	if(DEBUG)
 	{
 		cerr << "[+] g^(sk): ";
		printHex_G1(Cs);
		cerr << "[+] Cb*( Cs^sender_a()) : " ;
 		printHex_G1( Cb*( Cs^s.get_a()) );
	}
	start = clock();
 	if( !(pBs == ( Cb*( Cs^s.get_a()))) )
 	{
 		cerr << "[-][-][-]SK verification invalid[-][-][-]\nStopping transfer..." << endl;
 		exit(-1);
 		}
 	stop = clock();
	transfer_e = clock();
	double verification_time = (transfer_e - transfer_s) / (double)CLOCKS_PER_SEC;
 	s_ot_time += (stop - start) / (double)CLOCKS_PER_SEC;

 	if(DEBUG)
		cerr << endl << "Sk verification complete" << endl;

	if(DEBUG)
	{
		cerr << "---Starting OT protocol on file chunks---" << endl << endl;
    	cerr << "# of file sections : " << sec_par << endl;
   		cerr << "file section length : " << message_len << endl;
    	cerr << "Recipient bitstring (sk-R): " ;
    	for (int i = 0; i < sec_par; i++)
        cerr << bitstring[i];
	    cerr << endl;
	    cerr << endl;
	}
	double ot_time_v = 0;
	start = clock();
	for (int i = 0; i < sec_par; i++)
	{		
		OT_Recipient * ot_r = r.new_OT_Recipient(i);
        OT_Sender * ot_s = s.new_OT_Sender(i);		
		ot(ot_s, ot_r, e, public_g, i); 
	}
	stop = clock();
	ot_time_v = (stop - start) / (double)CLOCKS_PER_SEC;

	double join_time =0;
	start = clock();
	r.finish(); //r.temp_finish(); // needed to "stitch" together the char **message to image
	stop = clock();
	join_time = (stop - start) / (double)CLOCKS_PER_SEC;

	double detect_time =0;
	start = clock();
	detect_wm(bitstring, inputfile);
	stop = clock();
	detect_time = (stop - start) / (double)CLOCKS_PER_SEC;

	cerr << endl << "[+] Timing" << endl;
	cerr << "watermark: " << wm_time << endl;
	cerr << "***OT time***" << endl;
	cerr << "s_ot_time :" << s_ot_time << endl;
	cerr << "r_ot_time : " << r_ot_time << endl;
	cerr << "ot_time_v (= s_ot_time + r_ot_time - sk_verification) : " << ot_time_v << endl; 
	cerr << "join_time  : " << join_time << endl;
	cerr << "detect_time: " << detect_time << endl;
	cerr << "total verification time : " << verification_time << endl;
	totalTime = s_ot_time + r_ot_time + join_time + detect_time + wm_time;
	cerr << "Total :" <<  totalTime<< endl;
	
	char str[1024];
	sprintf(str,"rm /tmp/NDA/*");
	sysErr = system(str);
	
    if(sysErr < 0)
        cerr << "one of the system() or fgets() commands failed" << endl;
    cout << endl;
	

    return 0;  
}
