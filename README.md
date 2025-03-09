# Contiguous Memory Allocation

Gabriella (Gabi) Bekhrad  
gabimbek@gmail.com

## Overview/Source Files:
- allocator.c: C program managing a contiguous region of memory of given size, using different allocation strategies and keeping track of memory usage. Allows user to request, release, and compact memory using commands processed by the program.

## To compile:
gcc allocator.c -o allocator

## To run:
./allocator <initial amount of memory to allocate>

### Commands (sample commands):
- Request memory: RQ <process_name> <size> <strategy>  
    - For <strategy>: B (best-fit), F (first-fit), W (worst-fit)  
- Release memory: RL <process_name>  
- Display status: STAT  
- Compact memory: C  
- Exit: X  

## Sample Input/Output: 
./allocator 1000  
allocator>RQ p1 200 F  
Allocated 200 bytes for process p1 using F strategy.  
allocator>RQ p2 130 B  
Allocated 130 bytes for process p2 using B strategy.  
allocator>RQ p3 350 W  
Allocated 350 bytes for process p3 using W strategy.  
allocator>RQ p3 600 F  
Error: No block found for allocation.  
allocator>STAT  
Addresses [0:199] Process p1  
Addresses [200:329] Process p2  
Addresses [330:679] Process p3  
Addresses [680:999] Unused  
allocator>RL p1  
Released memory for process p1.  
allocator>STAT  
Addresses [0:199] Unused  
Addresses [200:329] Process p2  
Addresses [330:679] Process p3  
Addresses [680:999] Unused  
allocator>C  
Memory compaction completed.  
allocator>STAT  
Addresses [0:129] Process p2  
Addresses [130:479] Process p3  
Addresses [480:999] Unused  
allocator>X  
*program terminates*  
  
## Sources:
- https://stackoverflow.com/questions/67994197/how-do-i-define-a-struct-and-properly-allocate-memory-for-it-in-c
- https://www.geeksforgeeks.org/malloc-and-free-in-c/
- https://chatgpt.com/ (for debugging purposes)
- https://www.reddit.com/r/cprogramming/comments/oqjfkj/typedef_enum_vs_enum/
- https://www.tutorialspoint.com/c_standard_library/c_function_fflush.htm
- https://www.geeksforgeeks.org/format-specifiers-in-c/
- https://stackoverflow.com/questions/9069205/how-do-i-free-memory-in-c
- https://www.geeksforgeeks.org/c-program-to-move-a-string-in-memory/
- https://www.tutorialspoint.com/c_standard_library/c_function_atol.htm
