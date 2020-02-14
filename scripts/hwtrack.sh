#!/bin/bash
# author: Yaroslav Kashapov

if [ -z $1 ]
then	
	serial_speed=9600
	echo "Default serial speed: $serial_speed"
else
	serial_speed=$1
	echo "Serial speed: $serial_speed"
fi

char_devs=`find /dev/ -type c -name "*ttyUSB*" 2> /dev/null`
if [ $(echo "$char_devs" | wc -l) -eq 1 ]
then
	    port=$char_devs
	if [ -c $port ]
	then
        	echo "Autodetected $port"
	else 
		exit 0
	fi
else  
	port=0
	echo "Available serial ports:"
	echo "$char_devs"
	until [ -c $port ]
	do
        	echo -n "Serial port: "   
        	read port
        	if ! [ -c $port ]
        	then
                	echo "file $port doesn't exist. Please repeat"
        	fi      
	done
fi

stty $serial_speed
stty -F $port -hupcl

ram_total=`free -m | grep 'Mem' | awk '{print $2}'`
ssd_total=`df -h | grep -w "/" | awk '{print $2}'`
hdd_total=`df -h | grep -w /mnt | awk '{sum+=$2} END {print sum}'`

echo -n "Sending to $port..."
while [ -c $port ]
do
	sleep 1
	cpu_util=`top -b  -n1 | grep "Cpu(s)" | awk '{print $2 + $4}'`
	cpu_temp=`sensors | grep "Core 0:" | awk '{print $3}' | cut -c 2-3`
	ram_avail=`free -m | grep "Mem" | awk '{print $7}'`
	date=`date +"%H:%M"`
	ssd_use=`df -h | grep -w "/" | awk '{print $5}'`
	hdd_win=`df -h| grep -w "/mnt/windows" | awk '{print $3}' |  cut -c 1-2` 
	hdd_com=`df -h | grep -w "/mnt/common" | awk '{print $3}' | cut -c 1-2`
	hdd_use=$(($hdd_com+$hdd_win))
	echo   "$cpu_temp;$cpu_util;$ram_total;$ram_avail;$date;$ssd_total;$ssd_use;$hdd_total;$hdd_use;" > $port
done 2> /dev/null
