#/bin/bash
num=$(($1 - 1))
rand=$RANDOM
sec_par=$2

for i in $(seq 0 $num); do ./Testing_complete $i.pgm $sec_par $((${i} * rand)) ; cp out_complete.pgm $(( i + 1 )).pgm; mkdir -p /tmp/LIME_iter/${i}; cp /tmp/LIME/* /tmp/LIME_iter/${i};  done;

echo "===================================================== VERIFICATION ====================================================";

for i in $(seq 0 $num); do rm /tmp/LIME/*; cp /tmp/LIME_iter/${i}/* /tmp/LIME/; cp /tmp/LIME_iter/${num}/rec_out*pgm /tmp/LIME; cp ${num}.pgm out_complete.pgm ; ./Testing_ver_only $i.pgm $sec_par $(( $i * rand )) ; done
