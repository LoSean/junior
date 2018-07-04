#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
using namespace std;

#define SIZE 256
#define GREYSCALE 256



int main(int argc, char* argv[]) {
    FILE *file[3];

    if (!(file[0] = fopen(argv[1], "rb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    unsigned char imagedata[SIZE][SIZE];
    int j, k;
    int intensity[SIZE][2];
    memset(intensity, 0, 2 * SIZE * sizeof(int));
    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            imagedata[j][k] = fgetc(file[0]);
            intensity[(int)imagedata[j][k]][0]++;
        }
    fclose(file[0]);
    int min = SIZE;
    int last = 0;
    for (j = 0; j < SIZE; j++) {
        if (!intensity[j][0]) {
            continue;
        }
        else {
            if (min > j) min = j;
            intensity[j][0] += last;
            last = intensity[j][0];
        }
    }
#ifdef PRINT
    for (j = 0; j < SIZE; j++) {
        for (k = 0; k < SIZE; k++) {
            printf("%hhu ", imagedata[j][k]);
        }
        cout << endl;
    }
    printf("cdfmin = %d\n", intensity[min][0]);
    for (j = 0; j < SIZE; j++) {
        printf("intensity[%d][0] = %d\n", j, intensity[j][0]);
    }
#endif
    
    for (j = 0; j < SIZE; j++) {
        if (!intensity[j][0]) {
            continue;
        }
        else {
             int index = (int)round(
                                   (double)(intensity[j][0] - intensity[min][0])/(double)((SIZE * SIZE) - 1) * 
                                   (double)(GREYSCALE - 1)
                              );
#ifdef PRINT
             cout << "index = " << index << ", j = " << j << endl;
#endif
             intensity[j][1] = index;
        }
    }

#ifdef PRINT
    for (j = 0; j < SIZE; j++) {
        printf("intensity[%d][1] = %d\n", j, intensity[j][1]);
    }
#endif
    if (!(file[0] = fopen("sample2_out.raw", "wb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            fputc((unsigned char)intensity[(int)imagedata[j][k]][1], file[0]);
        }
    fclose(file[0]);
    return 0;
}
