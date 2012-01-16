#!/bin/bash

#for tom's input format
#
#DEPLOYMENT GUIDE:
#   1. make 'pagerank' binary
#   2. place this script with 'pagerank' program
#   3. issue command './tom.sh {dir}' to run pagerank, 
#      where '{dir}' is the data directory. 
#
#INPUT FILES:
#   * 'Wi.txt', 'Wj.txt', 'Ws.txt': triple of sparse matrix, 
#     where each a(i,j) = s
#   * 'Wz.txt': escape vector
#
#OUTPUT FILES:
#   * 'rank': reaching probability of random walk, one record
#      par line, with the format:
#      'id' '\t' 'rank'
#      where 'id' is the index in input matrix, starting from 1
#
#SUPER PARAMETERS:
#   * transfer ratio, see the last parameter of 'pagerank' 

if [[ $# == 1 ]] ; then
	dir=$1
else
	echo "usage:$0 {dir}"
	exit 255 
fi

cd $dir

#parameter calculation and format conversion

num=`echo -n \`wc -l Wz.txt | awk '{print $1}'\``
echo "total nodes:$num"

paste Wi.txt Wj.txt Ws.txt > link
seq 1 $num > seq
paste seq Wz.txt > escape-vector

#ulimit -c unlimited
../pagerank $num link escape-vector rank 0.9

sort -g -k2 -r rank > rank.sort

exit 0 
