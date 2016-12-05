#!/bin/bash
#echo $1
#echo $2
>input_file.txt
for (( c=0; c<800; c++ ))
do  
   echo "run 1 rdump" >> input_file.txt
done
	echo "quit" >> input_file.txt
	./$1 part-2/$2 <input_file.txt
