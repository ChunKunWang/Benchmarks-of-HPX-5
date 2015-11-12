#!/bin/sh
command_path="../"

command="hpx-5"

input=45

log[0]="log-help-hpx"

echo -e "Create new log file."

for i in 0 
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done


echo -e "${log[0]} is processing..."
for i in 0 2 4 6 8 10 16 20
do
	echo -e "\nThreshold:[$i]" >> ${log[0]}
	for j in {0..4}
	do
		${command_path}${command} --hpx-wfthreshold=$i $input >> ${log[0]}
	done
done

echo -e "Mission Complete!"

