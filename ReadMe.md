Name: Samiksha Sapkota
Date: February 18, 2026
Environment: Mac , Visual Studio Code
How to compile the project:
make
Example of how to run the project:
./oss -n 2 -s 2 -t 4.0 -i 0.3
on first run for the above example it will start the worker execution at 6 seconds and will end at around 10 seconds, but if you will run it again it will start at 0 and end at 4 seconds(as expected).

Program behaviour:
    I am creating a shared memory which is storing  value for custom/simulated clock which is created by oss.cpp. This clock is accessed by worker.cpp to check the time and display the output based on seconds and nanoseconds. Every 0.5 seconds oss.cpp also prints the process table which store data about the process it launched, such as PID,Occupied, Start Sec, Start Nano, End Sec, End Nano.

Generative AI used: ChatGPT and Google AI overview:
Prompt:
I'm trying to print a process table after every 0.5 seconds, I'm using a shared memory clock to check the current time and printing the table. it is printing after every 0.5 seconds but my program is running for 1000's of time. in my program I have -n option for number of process, -t for time to run the process with an interval of -i. So if I have -n 2 process with a -t 4.0 and -i 0.3 then I will have 2 process that will run for 4 seconds and the interval between those process will be 0.3 seconds. So the expected result should be 1st process starts at 0 seconds and 0nano seconds run upto 4 seconds 0nanoseconds, the second process will start at 0 seconds 300million nano seconds and ends at 4 seconds and 300 million nano second and terminate the program roughly around 4 seconds and 400-500 nano seconds, but program start executing the process at 7000 seconds sometime more than that. why is it taking 7000seconds to run.

Summary:
I was incrementing the clock aggressively by 1million and the incrementing function was at the start of a while loop. Once I decreased the incrementing value to much lower value of 10000, and moving my incrementing function to the end of while loop my program gave an expected output.

Prompt:
How do I calculate workers total runtime, can I simply add the endtime from processtable to calculate the total worker run time.

Summary:
No, The endTime's in process table also have interval time added to them so adding endtime will not give total runtime for workers. Instead n * t will give us the total run time.