#include <stddef.h>

int sum(int arr[], int n)
{
    int *ptest = NULL;
    int res = 0;

    *ptest = 1;
    for (int i = 0; i < n; i++)
        res += arr[i];
    return res;
}
