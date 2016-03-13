#!/bin/sh
command_path="../"
command="hybrid"

start=100
end=1000000000
step=10
seed=-1
flag_sort="xlcosb"
flag_seed=""

while getopts "i:o:a:r:" flag;
do      case $flag in
i)      seed=${OPTARG};;
o)	flag_sort=${OPTARG};;
        esac
done

if [ ${seed} = "-1" ]; then
	echo "no assigned seed"
	log[0]="log-nil"
	log[1]="log-as"
	log[2]="log-de"
else
	flag_seed="-i $seed"
	log[0]="log-nil_$seed"
	log[1]="log-as_$seed"
	log[2]="log-de_$seed"
fi


echo -e "Create new log file."

for i in {0..2}
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	echo -e "Create a new ${log[i]} file."
done

for ((j=$start; j<=$end; j*=$step))
do
	echo -e "\nInput:[$j]" >> ${log[0]}
	for k in {1..10}
	do
		${command_path}${command} -${flag_sort} ${flag_seed} $j >> ${log[0]}  
	done
done

#for ((j=$start; j<=$end; j*=$step))
#do
#	echo -e "\nInput:[$j]" >> ${log[1]}
#	for k in 0 1 2
#	do
#		${command_path}${command} -${flag_sort} -a ${flag_seed} $j >> ${log[1]}  
#	done
#done

#for ((j=$start; j<=$end; j*=$step))
#do
#	echo -e "\nInput:[$j]" >> ${log[2]}
#	for k in 0 1 2
#	do
#		${command_path}${command} -${flag_sort} -r ${flag_seed} $j >> ${log[2]}  
#	done
#done

echo -e "Mission Complete!"

