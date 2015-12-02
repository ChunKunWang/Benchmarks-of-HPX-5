#!/bin/sh
command_path="../"

command="hybrid"

input="-x -i 0 1000000000"

log="log-help-1"

echo -e "Create new log file."

if [ -f ${log} ]; then
	rm ${log}
#echo -e "Remove ${log} file."
fi
touch ${log}
#echo -e "Create a new ${log} file."

for i in 0 1 2 4 6 8 10 16 20 
do
	echo -e "\nTasks:[$j]" >> ${log}
		for j in {0..2}
		do
			${command_path}${command} --hpx-wfthreshold=$i --hpx-threads=20 ${input} >> ${log}
		done
done

echo -e "Mission Complete!"

