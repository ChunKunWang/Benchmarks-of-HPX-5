#!/bin/sh
command_path="../"
input_path="../input/"

command="basic"

input="medium.input"

log="log-basic"

echo -e "Create new log file."

if [ -f ${log} ]; then
	rm ${log}
#echo -e "Remove ${log} file."
fi
touch ${log}
#echo -e "Create a new ${log} file."

for k in {1..10}
do
	${command_path}${command} ${stacksize} ${input_path}${input} >> ${log}
done

echo -e "Mission Complete!"

