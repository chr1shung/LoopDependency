#include <stdio.h>

int main(void)
{
    int i, A[20], C[20] = {0}, D[20] = {1};

    for(i = 0; i < 20; i++) {
        A[i] = C[i];
        A[i] = D[i+2];
    }
}
