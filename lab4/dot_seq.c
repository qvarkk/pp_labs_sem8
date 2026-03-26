#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void* vmalloc(size_t size) {
    void* p = malloc(size);
    if (!p) {
        printf("malloc failed\n");
        exit(1);
    }
    return p;
}

int main(int argc, char **argv) {
    int N = 10000000;

    int* a = (int*)vmalloc((size_t)N * sizeof(int));
    int* b = (int*)vmalloc((size_t)N * sizeof(int));

    srand(time(NULL));

    for (int i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = rand() % 100;
    }

    double t0 = clock();

    long sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += (long)a[i] * (long)b[i];
    }

    double t1 = clock();

    printf("N   = %d\n", N);
    printf("Res = %lld\n", sum);
    printf("T   = %.6fs\n", (double)(t1 - t0) / CLOCKS_PER_SEC);

    free(a);
    free(b);
    return 0;
}