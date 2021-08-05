#include <iostream>
#include <sys/time.h>
#include "PBC.h"
#include "crypto.h" // has generic utility functions used by sender and receiver
#include "OT_Sender.h"
#include "OT_Recipient.h"
#include "Sender.h"
#include "Recipient.h"

using namespace std;


int main(int argc, char **argv)
{
  cout << endl;

  if (argc != 4){
    cerr << "Usage: " << argv[0] << " \"image.pgm\" \"sec_par\" \"rand_seed\" \n";
    return 1;
  }

  char str[256];
  sprintf(str, "mkdir /tmp/NDA");
  system(str);


  //const char *paramFileName = (argc > 1) ? argv[1] : "pairing.param";
  const char *paramFileName = "pairing.param";
  FILE *sysParamFile = fopen(paramFileName, "r");
  if (sysParamFile == NULL) 
  {
    cerr<<"Can't open the parameter file " << paramFileName << "\n";
    cerr<<"Usage: " << argv[0] << " [paramfile]\n";
    return 0;
  }

  //Seed ramdomness
  int rand_seed = atoi(argv[3]);
  srand( rand_seed);

  timeval tstart, tfinish, tstart2, tfinish2;
  double duration, duration2;
   
  // Initialize pairing
  Pairing e(sysParamFile);
  fclose(sysParamFile);  

  int sec_par = atoi(argv[2]);
  char *inputfile = argv[1];

  // Generate random watermarking bitstring
  int * bitstring;
  bitstring = (int *) malloc(sizeof(int) * sec_par);
  for (int i = 0; i < sec_par; i++){
    bitstring[i] = rand() % 2;
  }

  // Create protocol sender and recipient
  Sender s(inputfile,sec_par,e);
  Recipient r(bitstring,sec_par);

  // Generator g
  G1 g = G1(e,false);


  gettimeofday(&tstart, NULL);

  // Perform Oblivious transfer for all the parts
  for (int i = 0; i < sec_par; i++){
    OT_Sender * ot_s = s.new_OT_Sender(i);  
    OT_Recipient * ot_r = r.new_OT_Recipient(i);
    ot(ot_s, ot_r, e, g, i); 
  }

  r.finish();

  gettimeofday(&tfinish, NULL);
  duration = (tfinish.tv_sec - tstart.tv_sec) * 1000.0 ;
  duration += (tfinish.tv_usec - tstart.tv_usec) / 1000.0 ;

  cout << "OT: " << duration << endl;


  // Here the protocol is finished
  // Rest is evaluation
  
  gettimeofday(&tstart2, NULL);

  FILE *fp;
  char pout[256];
  char pout2[256];
  int errorcount = 0;

  // Check if all watermarks for the single parts could be detected correctly
  for (int i = 0; i < sec_par; i++){
    // Blind watermarking schemes do not need the orignal watermark for verification
    if (BLIND){
      sprintf(str, "watermarking/Meerwald/wm_%s_d -v 0 -s /tmp/LIME/%s_0.sig -o /tmp/LIME/%s_%d_0.wm /tmp/LIME/rec_out%d.pgm", WM_ALGO, WM_ALGO, WM_ALGO, i,i);
      system(str);
      sprintf(str, "watermarking/Meerwald/wm_%s_d -v 0 -s /tmp/LIME/%s_0.sig -o /tmp/LIME/%s_%d_1.wm /tmp/LIME/rec_out%d.pgm", WM_ALGO, WM_ALGO, WM_ALGO, i,i);
      system(str);
    }
    else{
      sprintf(str, "watermarking/Meerwald/wm_%s_d -v 0 -i /tmp/LIME/out%d.pgm -s /tmp/LIME/%s_0.sig -o /tmp/LIME/%s_%d_0.wm /tmp/LIME/rec_out%d.pgm", WM_ALGO, i,  WM_ALGO, WM_ALGO, i,i);
      system(str);
      sprintf(str, "watermarking/Meerwald/wm_%s_d -v 0 -i /tmp/LIME/out%d.pgm -s /tmp/LIME/%s_0.sig -o /tmp/LIME/%s_%d_1.wm /tmp/LIME/rec_out%d.pgm", WM_ALGO, i,  WM_ALGO, WM_ALGO, i,i);
      system(str);
    }


    sprintf(str, "watermarking/Meerwald/cmp_%s_sig -C -v 0 -s /tmp/LIME/%s_0.sig /tmp/LIME/%s_%d_0.wm", WM_ALGO,WM_ALGO,WM_ALGO, i);
    fp = popen(str, "r");
    fgets(pout, sizeof(pout) - 1, fp);
    double corr0 = atof(pout);
    pclose(fp);

    sprintf(str, "watermarking/Meerwald/cmp_%s_sig -C -v 0 -s /tmp/LIME/%s_1.sig /tmp/LIME/%s_%d_1.wm", WM_ALGO,WM_ALGO,WM_ALGO, i);
    fp = popen(str, "r");
    fgets(pout2, sizeof(pout2) - 1, fp);
    double corr1 = atof(pout2);
    pclose(fp);

    if (( corr0 < corr1) != bitstring[i]){
      // Count all wrongly detected bits
      errorcount++; 
    }
  } 

  // Verify that the overall watermark can be detected correctly
  if (BLIND){
  sprintf(str, "watermarking/Meerwald/wm_%s_d -s /tmp/LIME/%s.sig -o /tmp/LIME/%s.wm out_complete.pgm", WM_ALGO, WM_ALGO, WM_ALGO);
  }
  else{
  sprintf(str, "watermarking/Meerwald/wm_%s_d -i %s -s /tmp/LIME/%s.sig -o /tmp/LIME/%s.wm out_complete.pgm", WM_ALGO, inputfile, WM_ALGO, WM_ALGO);
  }
  system(str);
  sprintf(str, "watermarking/Meerwald/cmp_%s_sig -s /tmp/LIME/%s.sig /tmp/LIME/%s.wm", WM_ALGO,WM_ALGO,WM_ALGO);
  system(str);

  gettimeofday(&tfinish2, NULL);
  duration2 = (tfinish2.tv_sec - tstart2.tv_sec) * 1000.0 ;
  duration2 += (tfinish2.tv_usec - tstart2.tv_usec) / 1000.0 ;

  cout << "detection: " << duration2 << endl;

  if (errorcount)
    cout << endl << errorcount << " bits could not be detected correctly\n";
  else
    cout << endl << "All bits were detected correctly!\n";
  

  cout << endl;

  return 0; 

  
  return 0;
}
