#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
using namespace std;

#define SIZE 256

struct rgb {
	double r;
	double g;
	double b;
};

struct hsv {
	double h;
	double s;
	double v;
};

double my_mod(double num, double divider) {
    while(num >= divider) {
        num -= divider;
    }
    return num;
}

struct hsv rgb_to_hsv(struct rgb rgb) {
	struct hsv hsv;
	double min, max;

	min = (rgb.r < rgb.g) ? rgb.r: rgb.g;
	min = (min < rgb.b) ? min: rgb.b;
	max = (rgb.r > rgb.g) ? rgb.r: rgb.g;
	max = (max > rgb.b) ? max: rgb.b;

	hsv.v = max;
	if (hsv.v <= 0.0 || hsv.v == min) {
		hsv.s = 0.0;
		hsv.h = 0.0;
#ifdef PRINT
        cout << "zero" << endl;
#endif
	} 
	else {
		hsv.s = 1 - min / max;
        hsv.h = (rgb.r == max)? 60 * (rgb.g - rgb.b) / (max - min):
                (rgb.g == max)? 60 * (2 + (rgb.b - rgb.r) / (max - min)):
                                60 * (4 + (rgb.r - rgb.g) / (max - min));	

        if (hsv.h < 0) {
            hsv.h += 360;
        }
    }
	return hsv;
}

struct rgb hsv_to_rgb(struct hsv hsv) {
	struct rgb rgb;
	memset(&rgb, 0, sizeof(struct rgb));
	if (hsv.s == 0) {
		rgb.r = rgb.g = rgb.b = hsv.v;
	}
	else {
		double hi = hsv.h / 60;
        hi = (hi >= 6)? 0: hi;
        double c = hsv.v * hsv.s;
        double abs = (my_mod(hi, 2) - 1 >= 0)? my_mod(hi, 2) - 1: 1 - my_mod(hi, 2);
        double x = c * (1 - abs);
		switch((int)hi) {
			case 0:
				rgb.r = c;
				rgb.g = x;
				rgb.b = 0;
				break;
			case 1:
				rgb.r = x;
				rgb.g = c;
				rgb.b = 0;
				break;
			case 2:
				rgb.r = 0;
				rgb.g = c;
				rgb.b = x;
				break;
			case 3:
				rgb.r = 0;
				rgb.g = x;
				rgb.b = c;
				break;
			case 4:
				rgb.r = x;
				rgb.g = 0;
				rgb.b = c;
				break;
			case 5:
				rgb.r = c;
				rgb.g = 0;
				rgb.b = x;
				break;
			default: 
				break;
		}

        rgb.r += hsv.v - c;
        rgb.g += hsv.v - c;
        rgb.b += hsv.v - c;
	}
	return rgb;
}

int main(int argc, char* argv[]) {
	FILE *file[3];

	if (!(file[0] = fopen(argv[1], "rb"))) {
		cout << "Cannot open file!" << endl;
		exit(1);
	}

	unsigned char imagedata[SIZE][SIZE][3];
	int i, j, k;
	for (i = 0; i < 3; i++)
		for (j = 0; j < SIZE; j++)
			for (k = 0; k < SIZE; k++) {
				imagedata[j][k][i] = fgetc(file[0]);
			}
	fclose(file[0]);
#ifdef PRINT
	for (i = 0; i < 3; i++) {
		for (j = 0; j < SIZE; j++) {
			for (k = 0; k < SIZE; k++) {
				printf("%hhu ", imagedata[j][k][i]);
			}
			cout << endl;
		}
		cout << endl;
	}
#endif
#ifdef SPLIT
	file[0] = fopen("output1.raw", "wb");
	file[1] = fopen("output2.raw", "wb");
   	file[2] = fopen("output3.raw", "wb");

	for (i = 0; i < 3; i++) {
		for (j = 0; j < SIZE; j++)
			for (k = 0; k < SIZE; k++) {
				fputc(imagedata[j][k][i], file[i]);
				//fputc(imagedata[j][k][i], file[0]);
			}
	}
	fclose(file[0]);
    fclose(file[1]);
	fclose(file[2]);
#endif
	cout << "read data success"<< endl;
    double hsvdata[SIZE][SIZE][3];
    
	struct rgb rgbpixel;
	struct hsv hsvpixel;
	for (j = 0; j < SIZE; j++)
		for (k = 0; k < SIZE; k++) {
			rgbpixel.r = (double)imagedata[j][k][0]/255;
			rgbpixel.g = (double)imagedata[j][k][1]/255;
			rgbpixel.b = (double)imagedata[j][k][2]/255;
			hsvpixel = rgb_to_hsv(rgbpixel);
			hsvdata[j][k][0] = hsvpixel.h;
			hsvdata[j][k][1] = hsvpixel.s;
			hsvdata[j][k][2] = hsvpixel.v;
		}
    cout << "convert to hsv success" << endl;
    for (j = 0; j < SIZE; j++)
        for (k = 0; k < SIZE; k++) {
            hsvdata[j][k][0] += 330;
            hsvdata[j][k][0] = my_mod(hsvdata[j][k][0], 360);
            hsvdata[j][k][1] *= 1.5;
            hsvdata[j][k][1] = (hsvdata[j][k][1] > 1)? 1: hsvdata[j][k][1];
            hsvdata[j][k][2] *= 0.65;
            hsvdata[j][k][2] = (hsvdata[j][k][2] > 1)? 1: hsvdata[j][k][2];
        }
    cout << "adjust hue, saturation, and lightness success" << endl;
#ifdef PRINT
	for (i = 0; i < 3; i++) {
		for (j = 0; j < SIZE; j++) {
			for (k = 0; k < SIZE; k++) {
				printf("%lf ", hsvdata[j][k][i]);
			}
			cout << endl;
		}
		cout << endl;
	}
#endif
	for (j = 0; j < SIZE; j++)
		for (k = 0; k < SIZE; k++) {
			hsvpixel.h = hsvdata[j][k][0];
			hsvpixel.s = hsvdata[j][k][1];
			hsvpixel.v = hsvdata[j][k][2];
			rgbpixel = hsv_to_rgb(hsvpixel);
            if (rgbpixel.r == 0 && rgbpixel.g == 0 && rgbpixel.b == 0) {
                printf("hsvpixel.h = %lf, hsvpixel.s = %lf, hsvpixel.v = %lf\n", hsvdata[j][k][0], 
                                                                                 hsvdata[j][k][1],
                                                                                 hsvdata[j][k][2]);
            }
            imagedata[j][k][0] = (unsigned char)(rgbpixel.r * 255);
			imagedata[j][k][1] = (unsigned char)(rgbpixel.g * 255);
			imagedata[j][k][2] = (unsigned char)(rgbpixel.b * 255);
		}
    cout << "convert back to rgb success" << endl;
#ifdef PRINT
    for (i = 0; i < 3; i++) {
		for (j = 0; j < SIZE; j++) {
			for (k = 0; k < SIZE; k++) {
				printf("%hhu ", imagedata[j][k][i]);
			}
			cout << endl;
		}
		cout << endl;
	}
#endif
    file[0] = fopen("sample1_output.raw", "wb");
	for (i = 0; i < 3; i++) 
		for (j = 0; j < SIZE; j++)
			for (k = 0; k < SIZE; k++) {
				fputc(imagedata[j][k][i], file[0]);
			}
    fclose(file[0]);
    return 0;
}
