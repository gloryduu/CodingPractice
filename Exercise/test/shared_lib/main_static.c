#include <stdio.h>
#include <unistd.h>

int sum(int arr[], int n);

int g_test;
static int g_stest[2];
int arr[] = {1, 2};

int main(void)
{
    static int stest;
    int *pint = NULL;
    unsigned char test[8192];

    test[8191] = 255;
    printf("sum:%d\n", sum(arr, 2));
    while (1)
    {
        sleep(1);
    }

    return 0;
}
