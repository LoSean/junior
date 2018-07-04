#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <random>
#include <chrono>
#include <iostream>
using namespace std;

#define SIZE 256
#define GREYSCALE 256
#define ENLARGE 1

void add_noise(unsigned char imagedata[SIZE][SIZE]) {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator (seed);

    normal_distribution<double> distribution (0.0, 10.0);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            double afterNoise = (double)imagedata[i][j] + distribution(generator); 
            afterNoise = (afterNoise > 255)? 255: 
                         (afterNoise < 0)? 0:
                         afterNoise;
            imagedata[i][j] = (unsigned char)afterNoise;
        }
}

void pseudo_median(unsigned char imagedata[SIZE][SIZE]) {
    unsigned char enlarged[SIZE+2*ENLARGE][SIZE+2*ENLARGE];
    for (int i = 0; i < SIZE+2*ENLARGE; i++)
        for (int j = 0; j < SIZE+2*ENLARGE; j++) {
            if ((i < ENLARGE || i > SIZE+ENLARGE-1) && j > ENLARGE-1 && j < SIZE+ENLARGE) {
                enlarged[i][j] = (i < ENLARGE)? imagedata[0][j-ENLARGE]:
                                          imagedata[SIZE-1][j-ENLARGE];
            }
            else if ((j < ENLARGE || j > SIZE+ENLARGE-1) && i > ENLARGE-1 && i < SIZE+ENLARGE) {
                enlarged[i][j] = (j < ENLARGE)? imagedata[i-ENLARGE][0]:
                                          imagedata[i-ENLARGE][SIZE-1];
            }
            else if (i >= ENLARGE && i < SIZE+ENLARGE && j >= ENLARGE && j < SIZE+ENLARGE) {
                enlarged[i][j] = imagedata[i-ENLARGE][j-ENLARGE];
            }
        }
    int cMax, cMin, rMax, rMin;
    for (int i = ENLARGE; i < SIZE+ENLARGE; i++)
        for (int j = ENLARGE; j < SIZE+ENLARGE; j++) {
            for (int k = 0; k < ENLARGE+1; k++) {
                int cMaxTmp, cMinTmp, rMaxTmp, rMinTmp;
                for (int l = 0; l < ENLARGE+1; l++) {
                    if (!l) {
                        cMaxTmp = (int)enlarged[i+k-ENLARGE+l][j];
                        cMinTmp = (int)enlarged[i+k-ENLARGE+l][j]; 
                        rMaxTmp = (int)enlarged[i][j+k-ENLARGE+l]; 
                        rMinTmp = (int)enlarged[i][j+k-ENLARGE+l]; 
                    }
                    else {
                        cMaxTmp = (cMaxTmp < (int)enlarged[i+k-ENLARGE+l][j])? cMaxTmp: 
                                                                               (int)enlarged[i+k-ENLARGE+l][j];
                        cMinTmp = (cMinTmp > (int)enlarged[i+k-ENLARGE+l][j])? cMinTmp: 
                                                                               (int)enlarged[i+k-ENLARGE+l][j];
                        rMaxTmp = (rMaxTmp < (int)enlarged[i][j+k-ENLARGE+l])? rMaxTmp: 
                                                                               (int)enlarged[i][j+k-ENLARGE+l];
                        rMinTmp = (rMinTmp > (int)enlarged[i][j+k-ENLARGE+l])? rMinTmp: 
                                                                               (int)enlarged[i][j+k-ENLARGE+l];
                    }
                }
                if (!k) {
                    cMax = cMaxTmp;
                    cMin = cMinTmp;
                    rMax = rMaxTmp;
                    rMin = rMinTmp;
                } 
                else {
                    cMax = (cMax > cMaxTmp)? cMax: cMaxTmp;
                    cMin = (cMin < cMinTmp)? cMin: cMinTmp;
                    rMax = (rMax > rMaxTmp)? rMax: rMaxTmp;
                    rMin = (rMin < rMinTmp)? rMin: rMinTmp;
                }
            }
            int pemdMax = (cMax > rMax)? cMax: rMax;
            int pemdMin = (cMin < rMin)? cMin: rMin;
            imagedata[i-ENLARGE][j-ENLARGE] = (unsigned char)((pemdMax + pemdMin)/2); 
        }
}

double getPSNR(unsigned char imagedata[SIZE][SIZE], unsigned char imageBackup[SIZE][SIZE]) {
    double mse = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            double difference = (double)(imagedata[i][j] - imageBackup[i][j]);
            mse += difference * difference;
        }
    mse /= SIZE * SIZE;

    return 10 * log10((GREYSCALE - 1) * (GREYSCALE - 1) / mse);
}

int main(int argc, char* argv[]) {
    FILE *file[3];

    if (!(file[0] = fopen(argv[1], "rb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    unsigned char imagedata[SIZE][SIZE];
    int j, k;
    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            imagedata[j][k] = fgetc(file[0]);
        }
    fclose(file[0]);

    unsigned char imageBackup[SIZE][SIZE];
    memcpy(imageBackup, imagedata, sizeof(unsigned char) * SIZE * SIZE);
    add_noise(imagedata);
    
    if (!(file[0] = fopen("sample4_noise.raw", "wb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            fputc(imagedata[j][k], file[0]);
        }
    fclose(file[0]);

    pseudo_median(imagedata);    
    cout << getPSNR(imagedata, imageBackup) << endl;

    if (!(file[0] = fopen("sample4_pseudo.raw", "wb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            fputc(imagedata[j][k], file[0]);
        }
    fclose(file[0]);
    
    pseudo_median(imagedata);    

    cout << getPSNR(imagedata, imageBackup) << endl;
    if (!(file[0] = fopen("sample4_clear2.raw", "wb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            fputc(imagedata[j][k], file[0]);
        }
    fclose(file[0]);

    for (int i = 0; i < 5; i++) {
        pseudo_median(imagedata);    
        cout << getPSNR(imagedata, imageBackup) << endl;
    }
    if (!(file[0] = fopen("sample4_clear3.raw", "wb"))) {
        cout << "Cannot open file!" << endl;
        exit(1);
    }

    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            fputc(imagedata[j][k], file[0]);
        }
    fclose(file[0]);
    return 0;
}
