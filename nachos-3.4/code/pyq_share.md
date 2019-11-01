Creating user program thread 1
Creating user program thread 2
Forking thread "User_program_1" with func = 0x804ede0, arg = 1
Putting thread User_program_1 on ready list.
Forking thread "User_program_2" with func = 0x804ede0, arg = 2
Putting thread User_program_2 on ready list.
Yielding thread "main"
Putting thread main on ready list.
Switching from thread "main" to thread "User_program_1"
Running user program thread 1
======== addrspace information ==========
numPages = 11
VPN	PPN	valid	readOnly	use	dirty
0	0	1	0	0	0	
1	1	1	0	0	0	
2	2	1	0	0	0	
3	3	1	0	0	0	
4	4	1	0	0	0	
5	5	1	0	0	0	
6	6	1	0	0	0	
7	7	1	0	0	0	
8	8	1	0	0	0	
9	9	1	0	0	0	
10	10	1	0	0	0	
=========================================
TLBSize=4, TLB Miss: 4, TLB Hit: 28, Total Translation: 32, TLB Miss Rate: 12.50%
User Program Exit With Status 87
Finishing thread "User_program_1"
Sleeping thread "User_program_1"
Switching from thread "User_program_1" to thread "User_program_2"
Running user program thread 2
======== addrspace information ==========
numPages = 11
VPN	PPN	valid	readOnly	use	dirty
0	11	1	0	0	0	
1	12	1	0	0	0	
2	13	1	0	0	0	
3	14	1	0	0	0	
4	15	1	0	0	0	
5	16	1	0	0	0	
6	17	1	0	0	0	
7	18	1	0	0	0	
8	19	1	0	0	0	
9	20	1	0	0	0	
10	21	1	0	0	0	
=========================================
TLBSize=4, TLB Miss: 4, TLB Hit: 55, Total Translation: 59, TLB Miss Rate: 6.78%
User Program Exit With Status 87
Finishing thread "User_program_2"
Sleeping thread "User_program_2"
Switching from thread "User_program_2" to thread "main"
Now in thread "main"
Deleting thread "User_program_2"
Finishing thread "main"
Sleeping thread "main"
No threads ready or runnable, and no pending interrupts.
Assuming the program completed.
Machine halting!

Ticks: total 104, idle 0, system 60, user 44
Disk I/O: reads 0, writes 0
Console I/O: reads 0, writes 0
Paging: faults 0
Network I/O: packets received 0, sent 0

Cleaning up...

