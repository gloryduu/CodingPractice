#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

extern int arr1[];
//int sum(int arr[], int n);
int arr[] = {1, 2, 3};

int main(void)
{
    void *handle = NULL;
    int (*psum)(int arr[], int n) = NULL;
    char *error = NULL;

    handle = dlopen("./libsum.so", RTLD_LAZY);
    if (!handle)
    {
        printf("dlopen error: %s\n", dlerror());
        return -1;
    }

    psum = dlsym(handle, "sum");
    if (!psum)
    {
        printf("dlsym error: %s\n", dlerror());
        return -1;
    }

    printf("sum: %d\n", psum(arr1, 2));
    printf("sum: %d\n", psum(arr, 3));

    if (dlclose(handle))
    {
        printf("dlclose error: %s\n", dlerror());
        return -1;
    }

    while (1)
    {
        sleep(1);
    }

    return 0;
}
