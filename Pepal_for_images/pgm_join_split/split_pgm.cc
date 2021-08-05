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
	int width;
	int height;
	int max_grey;
	int width_split;
	int height_split;

	if (argc !=4){
		cerr << "Usage : " << argv[0] << " source.pgm width_split height_split" << endl;
		return 1;
	}
	ifstream orig (argv[1]);
	if (!orig.is_open()){
		cerr << "Could not open file \n";
		return 1;
	}
	
	width_split=atoi(argv[2]);
	height_split=atoi(argv[3]);

	char ident[2];
	orig.read(ident,2);
	if (ident[0] != 'P' || ident[1] !='5'){
		cerr << "Wrong file format in split\n";
		return 1;
	}

	char x = ' ';
	char *buffer = &x;
	get_next_non_space(buffer, &orig);
		
	char width_array[10];
	width_array[0]=x;
	read_number(width_array, &orig);
	width = atoi(width_array);


	get_next_non_space(buffer, &orig);

	char height_array[10];
	height_array[0]=x;
	read_number(height_array, &orig);
	height = atoi(height_array);
	
	get_next_non_space(buffer, &orig);

	char max_grey_array[10];
	max_grey_array[0]=x;
	read_number(max_grey_array, &orig);
	max_grey = atoi(max_grey_array);

	if (max_grey > 255){
		cerr << "max_grey value not supported (>255) '\n'";
	}
	
	char *content; 
	content = (char *) malloc(sizeof(char) * width*height);
	orig.read(content, width*height);

	orig.close();

	//ofstream *out;
	//out = (ofstream *) malloc(sizeof(ofstream) * n_split);
	
	for (int i = 0; i <  height_split; i++){
		for (int j = 0; j <  width_split; j++){
			char name[20];
			sprintf( name, "/tmp/NDA/out%d.pgm", i * width_split + j);

			ofstream  out(name);
			out << "P5 " << width / width_split << " " << height / height_split << " " << max_grey << '\n';

			int upper_left = width * (height/height_split) * i  + (width / width_split) * j ; 

			for(int k = 0; k < height / height_split; k ++){
				for(int l = 0; l < width / width_split; l ++){
					out << content[upper_left + l +  width * k]; 
				}
			}
			//cerr << "1" << endl;
			out.close();
		}
	}
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

