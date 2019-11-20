/* pyq_test.c 
 *    Test program
 *
 *    Get sum() from 0 to N
 *
 */
#include "syscall.h"

#define N 3
int
main()
{
    int i;
    int sum = 0;
    for (i = 0; i <= N; i++){
        sum += i;
    }
    Exit(sum);
}
