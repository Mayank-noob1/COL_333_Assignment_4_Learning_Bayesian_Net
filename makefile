run: 
	time g++ format_checker.cpp -o check
	time ./compile.sh
	time ./run.sh Alarm.bif records.dat
	time ./check