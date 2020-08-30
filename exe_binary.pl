#! /usr/bin/perl -w
use List::Util qw[min max];

@t1=(8,9,10,11);
@t2=(8,7,6,5);
# $t=200000;
# print "${t}_${t1[0]}_${t2[0]}_result\n";
# $i=5;
for($t=100000; $t <100000; $t=$t+10000)
{
 	for($i=0; $i<4 ; $i=$i+1)
	{
		for($k=0; $k<20;$k++)
		{
			system "./NN-chain ../p100n.txt $t 264 $t1[$i] $t2[$i] >> PROT_${t}_${t1[$i]}_${t2[$i]}_result";
 			system "./NN-chain ../2000000-1000.txt $t 128 $t1[$i] $t2[$i] >> RND_${t}_${t1[$i]}_${t2[$i]}_result";
# 			system "./NN-chain ../MiniBooNE_PID.txt 130065 50 $t1[$i] $t2[$i] >> UCI5_${t1[$i]}_${t2[$i]}_result";
# 			system "./NN-chain ../synthetic 100000 10 $t1[$i] $t2[$i] >> UCI4_${t1[$i]}_${t2[$i]}_result";
# 			system "./NN-chain ../slice.txt 53500 385 $t1[$i] $t2[$i] >> UCI2_${t1[$i]}_${t2[$i]}_result";
# 			system "./NN-chain ../magic04.txt 19020 10 $t1[$i] $t2[$i] >> UCI1_${t1[$i]}_${t2[$i]}_result";
# 			system "./NN-chain ../Candidate.txt 41421 18 $t1[$i] $t2[$i] >> SMD_${t1[$i]}_${t2[$i]}_result";
# 			system "./NN-chain ../Reaction.txt 65554 28 $t1[$i] $t2[$i] >> UCI3_${t1[$i]}_${t2[$i]}_result";
		}
	}
}
