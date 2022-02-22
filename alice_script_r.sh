#!/bin/bash

if [ $# -lt 2 ];
then
    echo "USAGE: ./alice_script.sh bin_path input_path OR ./alice_script.sh bin_path input_path output_path"
    exit 0
fi

bin_path=$1
input_path=$2

for file in `ls $input_path`
do
	for ((a =1;a<=3;a++))
	do
		echo "$input_path/$file"
		if [ $# -eq 3 ];
		then
			$bin_path 1 3351 $input_path/$file $3/$file
		else
			$bin_path 1 3351 $input_path/$file
		fi
	done
done