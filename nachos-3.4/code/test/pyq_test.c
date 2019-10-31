/* pyq_test.c 
 *    Test program to print Hello World.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 *
 *  For Lab3 Memory
 */
#include "syscall.h"

#define N 20

int
main()
{
    int i;
    int result[N];
    result[0] = 0;
    result[1] = 1;
    for (i = 2; i < N; i++)
    {
        result[i] = result[i-1] + result[i-2];
    }
    Exit(result[N-1]);
}
