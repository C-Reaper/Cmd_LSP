#include <stdio.h>

int Add(int a, int b)
{
    return a + b;
}

void Hello(void)
{
    printf("Hello\n");
}

int main(void)
{
    int value = Add(10, 20);

    Hello();

    printf("%d\n", value);

    does_not_exist();

    return 0;
}