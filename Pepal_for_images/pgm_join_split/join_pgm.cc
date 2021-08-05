/*
 * =====================================================================================
 *
 *       Filename:  split_pgm.cc
 *
 *    Description:  code to split pgm files
 *
 *        Version:  1.0
 *        Created:  04/13/2013 09:27:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>

using namespace std;

void get_next_non_space(char *buffer, ifstream *orig);
void read_number(char num[], ifstream *orig);
bool is_square(int x);

int main(int argc, char *argv[]){
	int height_new;
	int width_new;
	int n_split;
	int sqrt_n;
	bool first = true;
	char * content_new;


	if (argc !=3){
		cerr << "Usage : " << argv[0] << " source n_split" << endl;
		return 1;
	}
	
	n_split=atoi(argv[2]);
	if (!is_square(n_split)){
		cerr << "n_split must be square \n";	
		return 1;
	}
	sqrt_n = sqrt(n_split);

	ofstream out ("NDA_out_complete.pgm");

	for (int i = 0; i<n_split; i++){

		char name[32];
		sprintf(name, "%s%d.pgm", argv[1], i);

		ifstream file (name);

		if (!file.is_open()){
			cerr << "Could not open " << name << endl;
			return 1;
		}

		int width;
		int height;
		int max_grey;

		char ident[2];
		file.read(ident,2);
		if (ident[0] != 'P' || ident[1] !='5'){
			cerr << "Wrong file format \n";
			return 1;
		}

		char x = ' ';
		char *buffer = &x;
		get_next_non_space(buffer, &file);
		
		char width_array[10];
		width_array[0]=x;
		read_number(width_array, &file);
		width = atoi(width_array);


		get_next_non_space(buffer, &file);

		char height_array[10];
		height_array[0]=x;
		read_number(height_array, &file);
		height = atoi(height_array);
		
		get_next_non_space(buffer, &file);

		char max_grey_array[10];
		max_grey_array[0]=x;
		read_number(max_grey_array, &file);
		max_grey = atoi(max_grey_array);

		if (max_grey > 255){
			cerr << max_grey <<" max_grey value not supported (>255) \n";
		}

	
		char *content; 
		content = (char *) malloc(sizeof(char) * width*height);
		file.read(content, width*height);

		file.close();


		if (first){
			out << "P5 " << width * sqrt_n << " " << height * sqrt_n << " " << max_grey << '\n';
			first = false;
			height_new = height * sqrt_n;
			width_new = width * sqrt_n;
			content_new = (char *) malloc(sizeof (char) * height_new * width_new);
		}


		int upper_left = width_new * height * (i / sqrt_n)  + width *  (i % sqrt_n); 

		for (int k = 0; k < height; k++){
			for (int l = 0; l < width; l++){
				content_new[upper_left+ l + k * width_new] = content[k*width + l];	
			}
		}


	}
	for (int i = 0; i < width_new * height_new ; i++){
		out << content_new[i];
	}
	out.close();

	//ofstream *out;
	//out = (ofstream *) malloc(sizeof(ofstream) * n_split);
	
//	for (int i = 0; i < n_split; i++){
//		char name[12] = "out0000.pgm";
//		name[3] = '0' + i / 1000;
//		name[4] = '0' + (i % 1000) / 100;
//		name[5] = '0' + (i % 100) /10;
//		name[6] = '0' + i % 10;
//
//		ofstream  out(name);
//		out << "P5 " << width / sqrt_n << " " << height / sqrt_n << " " << max_grey << '\n';
//
//		int upper_left = width * ((height/sqrt_n) * (i / sqrt_n))  + (width / sqrt_n) *  (i % sqrt_n); 
//
//		for(int j = 0; j < width / sqrt_n; j ++){
//			for(int k = 0; k < height / sqrt_n; k ++){
//				out << content[upper_left + k +  width * j]; 
//			}
//		}
//
//		out.close();
//	}

	return 0;
}

void get_next_non_space(char *buffer, ifstream *orig){
	orig->read(buffer,1);
	char x = *buffer;
	while (x == ' ' || x == '\n' || x == '\r' || x == '\t'){
		orig->read(buffer,1);
		x = *buffer;
	}
}

void read_number(char num[], ifstream *orig){
	int counter = 1;
	char x = num[0];
	char * buffer = &x;
	while (!(x == ' ' || x == '\n' || x == '\r' || x == '\t')){
		orig->read(buffer,1);
		x = *buffer;
		num[counter] = x;
		counter++;
	}
	num[counter] = '\0';
}

bool is_square(int x){
	int tmp = (int) sqrt(x);
	return (x == tmp*tmp);
}

