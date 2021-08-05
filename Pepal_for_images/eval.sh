#!/bin/bash

echo "Evaluating the values";
rm eval/result/*

for type in watermarking signature encryption ot detection; 
do
  echo "Evaluating ${type}";
  for size in cameraman.pgm lena.pgm man.pgm moto.pgm ;
  do
    case ${size} in
      cameraman.pgm)
        pixels=65536 ;;
      lena.pgm)
        pixels=263144 ;;
      man.pgm)
        pixels=1048576 ;;
      moto.pgm)
        pixels=4194304 ;;
    esac
    
    mean=$(awk '{sum+=$2} END {printf "%f",(sum/NR)}' eval/${size}.${type});
    dev=$(awk '{sum+=$2; array[NR]=$2} END {for(x=1;x<=NR;x++){sumsq+=((array[x]-(sum/NR))**2);}printf "%f", sqrt(sumsq/NR)}' eval/${size}.${type});
    echo -e "${pixels} \t ${mean} \t ${dev}" >> eval/result/size.${type} 
  done;
done;

for type in watermarking signature encryption ot detection; 
do
  echo "Evaluating ${type}";
  for sec_par in 1 4 16 64 256; 
  do
    mean=$(awk '{sum+=$2} END {printf "%f",(sum/NR)}' eval/${sec_par}.${type});
    dev=$(awk '{sum+=$2; array[NR]=$2} END {for(x=1;x<=NR;x++){sumsq+=((array[x]-(sum/NR))**2);}printf "%f", sqrt(sumsq/NR)}' eval/${sec_par}.${type});
    echo -e "${sec_par} \t ${mean} \t ${dev}" >> eval/result/sec_par.${type}

  done;
done;

