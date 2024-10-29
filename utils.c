#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "utils.h"

#define GAUSSIAN_KERNEL_SIZE 5

typedef unsigned char gray;

float gaussian_kernel[GAUSSIAN_KERNEL_SIZE][GAUSSIAN_KERNEL_SIZE] = {
    {1.0/273.0, 4.0/273.0, 6.0/273.0, 4.0/273.0, 1.0/273.0},
    {4.0/273.0, 16.0/273.0, 24.0/273.0, 16.0/273.0, 4.0/273.0},
    {6.0/273.0, 24.0/273.0, 36.0/273.0, 24.0/273.0, 6.0/273.0},
    {4.0/273.0, 16.0/273.0, 24.0/273.0, 16.0/273.0, 4.0/273.0},
    {1.0/273.0, 4.0/273.0, 6.0/273.0, 4.0/273.0, 1.0/273.0}
};

char pm_getc(FILE *file) {
    int ich;
    char ch;

    ich = getc(file);
    if (ich == EOF)
        pm_erreur("EOF / read error");
    ch = (char)ich;

    if (ch == '#') {
        do {
            ich = getc(file);
            if (ich == EOF)
                pm_erreur("EOF / read error");
            ch = (char)ich;
        } while (ch != '\n' && ch != '\r');
    }

    return ch;
}

bit pm_getbit(FILE *file) {
    char ch;

    do {
        ch = pm_getc(file);
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

    if (ch != '0' && ch != '1')
        pm_erreur("read error : != [0|1]");

    return (ch == '1') ? 1 : 0;
}

unsigned char pm_getrawbyte(FILE *file) {
    int iby;

    iby = getc(file);
    if (iby == EOF)
        pm_erreur("EOF / read error ");
    return (unsigned char)iby;
}

int pm_getint(FILE *file) {
    char ch;
    int i;

    do {
        ch = pm_getc(file);
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

    if (ch < '0' || ch > '9')
        pm_erreur("read error : != [0-9]");

    i = 0;
    do {
        i = i * 10 + ch - '0';
        ch = pm_getc(file);
    } while (ch >= '0' && ch <= '9');

    return i;
}

void pm_erreur(char *texte) {
    fprintf(stderr, "\n%s \n\n", texte);
    exit(1);
}

void compute_gradient_x(gray *image, gray *grad_x, int rows, int cols) {
    int scharr_x[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};

    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            int sum = 0;
            for (int u = -1; u <= 1; u++) {
                for (int v = -1; v <= 1; v++) {
                    sum += image[(i + u) * cols + (j + v)] * scharr_x[u + 1][v + 1];
                }
            }
            grad_x[i * cols + j] = (gray)abs(sum); // store absolute gradient
        }
    }
}

void compute_gradient_y(gray *image, gray *grad_y, int rows, int cols) {
    int scharr_y[3][3] = {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}};

    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            int sum = 0;
            for (int u = -1; u <= 1; u++) {
                for (int v = -1; v <= 1; v++) {
                    sum += image[(i + u) * cols + (j + v)] * scharr_y[u + 1][v + 1];
                }
            }
            grad_y[i * cols + j] = (gray)abs(sum); // store absolute gradient
        }
    }
}

void compute_gradient_magnitude(gray *grad_x, gray *grad_y, gray *magnitude, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            magnitude[i * cols + j] = (gray)sqrt(grad_x[i * cols + j] * grad_x[i * cols + j] + grad_y[i * cols + j] * grad_y[i * cols + j]);
        }
    }
}

void gaussian_filter(gray *input, gray *output, int rows, int cols) {
    int offset = GAUSSIAN_KERNEL_SIZE / 2;

    // Clear output image to ensure all pixels are initialized
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            output[i * cols + j] = 0; // Initialize output pixel
        }
    }

    // Apply Gaussian filter to the image
    for (int i = offset; i < rows - offset; i++) {
        for (int j = offset; j < cols - offset; j++) {
            float sum = 0.0;
            for (int u = -offset; u <= offset; u++) {
                for (int v = -offset; v <= offset; v++) {
                    sum += input[(i + u) * cols + (j + v)] * gaussian_kernel[u + offset][v + offset];
                }
            }
            output[i * cols + j] = (gray)(sum + 0.5); // Rounding to avoid truncation issues
        }
    }
}

void non_maximum_suppression(gray *magnitude, gray *grad_x, gray *grad_y, gray *output, int rows, int cols) {
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            // Calculate angle of gradient
            float angle = atan2(grad_y[i * cols + j], grad_x[i * cols + j]) * (180 / M_PI);
            if (angle < 0) angle += 180;

            // Compare with neighbors
            int q = 255, r = 255;
            if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180)) {
                q = magnitude[i * cols + (j + 1)];
                r = magnitude[i * cols + (j - 1)];
            } else if (22.5 <= angle && angle < 67.5) {
                q = magnitude[(i + 1) * cols + (j + 1)];
                r = magnitude[(i - 1) * cols + (j - 1)];
            } else if (67.5 <= angle && angle < 112.5) {
                q = magnitude[(i + 1) * cols + j];
                r = magnitude[(i - 1) * cols + j];
            } else if (112.5 <= angle && angle < 157.5) {
                q = magnitude[(i - 1) * cols + (j + 1)];
                r = magnitude[(i + 1) * cols + (j - 1)];
            }

            // Suppress non-maximum values
            if (magnitude[i * cols + j] >= q && magnitude[i * cols + j] >= r) {
                output[i * cols + j] = magnitude[i * cols + j];
            } else {
                output[i * cols + j] = 0;
            }
        }
    }
}

void hysteresis_thresholding(gray *output, gray *result, int rows, int cols, int low_thresh, int high_thresh) {
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            if (output[i * cols + j] >= high_thresh) {
                result[i * cols + j] = 255; // Strong edge
            } else if (output[i * cols + j] >= low_thresh) {
                // Check if connected to a strong edge
                if ((output[(i - 1) * cols + j - 1] >= high_thresh) || 
                    (output[(i - 1) * cols + j] >= high_thresh) || 
                    (output[(i - 1) * cols + j + 1] >= high_thresh) || 
                    (output[i * cols + j - 1] >= high_thresh) || 
                    (output[i * cols + j + 1] >= high_thresh) || 
                    (output[(i + 1) * cols + j - 1] >= high_thresh) || 
                    (output[(i + 1) * cols + j] >= high_thresh) || 
                    (output[(i + 1) * cols + j + 1] >= high_thresh)) {
                    result[i * cols + j] = 255; // Weak edge connected to strong edge
                } else {
                    result[i * cols + j] = 0; // Suppress
                }
            } else {
                result[i * cols + j] = 0; // Suppress
            }
        }
    }
}