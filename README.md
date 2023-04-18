# 180-lab-wk5
This repository contains code pertaining to exercises showing the features of Code Affinity in the C programming languages in the subject of CMSC 180 - Parallel Programming

# CMSC 180 - Week 5 - Lab 03
## Author
Author: Aron Resty Ramillano | 2020-01721
Section: T3L

## App Description
An implementation of multithreading in C by interpolating pointsi n an n x n matrix, with the use of `<pthreads.h>` in C, and in addition, implementing cpu affinity that is included in the pthreads library.

## Features
 1. It can calculate matrices that is divisible by 10 with high performance using the C language
 2. This revision has a benchmark that can test the program's performance with 3 passes on calculating matrices ranging from 100 to 20000
 3. It can calculate high numbers of matrices with the help of threading to speedup the calculation by subdividing it into smaller submatrices
 4. It can also benchmark itself automatically with predetermined n and t values.
 5. It can now assign threads to specific CPUs.

## Files included:
 - lab02-1.c
 - README.md
  
## How to setup
 - Must have GCC
 - Run this command:
	 - `gcc -o a lab03.c -lm -pthread; ./a`
	 - The `-lm` enables the `<math.h>` library for use of the `ceil` and `floor` functions
     - The `-pthread` enables the `<pthread.h>` library in my machine. It may work perfectly on your machine, but my machine requires these arguments

# Resources
- https://www.geeksforgeeks.org/multidimensional-arrays-c-cpp/
- https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
- https://stackoverflow.com/questions/5201708/how-to-return-a-2d-array-from-a-function-in-c
- https://stackoverflow.com/questions/2150291/how-do-i-measure-a-time-interval-in-c
- https://man7.org/linux/man-pages/man3/pthread_setaffinity_np.3.html
- https://bytefreaks.net/programming-2/cc-set-affinity-to-threads-example-code
- https://stackoverflow.com/questions/1407786/how-to-set-cpu-affinity-of-a-particular-pthread
- https://man7.org/linux/man-pages/man3/CPU_SET.3.html
