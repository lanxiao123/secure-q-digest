#!/bin/bash

if [ $# -lt 3 ];
then
    echo "USAGE: ./alice_script.sh bin_path merge_output_path1 merge_output_path2"
    exit 0
fi

bin_path=$1
output_path_1=$2
output_path_2=$3

for file in `ls $output_path_1`
do
    echo "$output_path_1/$file compares with $output_path_2/$file result:" 
    $bin_path $output_path_1/$file $output_path_2/$file
done

