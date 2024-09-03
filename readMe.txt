-This code runs based on the fact that all inputs are in the same form, meaning the first 7 lines are the given inputs, the next lines are determined on how many processes there are and each line is the process number followed by page size.
-This code runs based on the fact that the input for a process ends with its process number followed by a "-1".
-This code runs on the fact that there is only 6 algorithms we need to run and use.
-This code has not been written to attempt any of the extra credit. It is only the base assignment
-This code uses a seperate header file called "LinkedList.h". The code will not run whatsoever if this file is not in the same folder as the main.cpp
-This file will only run if the test inputs are in the same folder as main.cpp
-The given input files I will put on the server in my zipped folder that I submit
-The program runs if you use the commands in a fresh new file, but I have already done the first command, so all you need to do to run the code is run the second command. The second command needs to be entered everytime there is a new input file. If there are errors like "undefined reference to sem_wait", then the first command needs to be entered.
        g++ -pthread -o myprog main.cpp
        ./myprog Assignment3-Input.txt     