#include <stdio.h>

int main ()
{
    for (int i = 0; ; i += 10){
        printf("%d % 500 = %d\n",i, i % 500);
    }
}