#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "utils.h"

// Define gray if it's not in utils.h
typedef unsigned char gray;

int main(int argc, char **argv) {
    if (argc != 5) { // Ensure correct argument count
        fprintf(stderr, "Usage: %s <input.pgm> <output.pgm> <low_threshold> <high_threshold>\n", argv[0]);
        return 1;
    }

      //int threshold;

    // Ensure there are 3 arguments: input file, output file
    // if (argc != 3) 
    // {
    //     printf("\nUsage: %s file_in file_out\n\n", argv[0]);
    //     exit(0);
    // }

    // Parse threshold from command line
    // threshold = atoi(argv[3]); 
    // if (threshold < 0 || threshold > 255) {
    //     printf("Threshold must be between 0 and 255.\n");
    //     exit(1);
    // }

    // Open input file

    // Open the input file in binary mode
    FILE *infile = fopen(argv[1], "rb");
    if (!infile) {
        perror("Error opening input file");
        fprintf(stderr, "Please check if the file exists and path is correct: %s\n", argv[1]);
        return 1;
    }

    // Open the output file in binary mode
    FILE *outfile = fopen(argv[2], "wb");
    if (!outfile) {
        perror("Error opening output file");
        fclose(infile);
        return 1;
    }

    int rows, cols, maxval;
    char magic[3];

    // Read the PGM header and check format
    if (fscanf(infile, "%2s\n", magic) != 1) {
        fprintf(stderr, "Error reading magic number. This program only supports PGM formats.\n");
        fclose(infile);
        fclose(outfile);
        return 1;
    }

    // Skip over comments in the header
    int c;
    while ((c = fgetc(infile)) == '#') {
        while ((c = fgetc(infile)) != '\n' && c != EOF);
    }
    ungetc(c, infile);

    // Read dimensions and maxval
    if (fscanf(infile, "%d %d\n%d\n", &cols, &rows, &maxval) != 3) {
        fprintf(stderr, "Error reading PGM header. Ensure the file has valid dimensions.\n");
        fclose(infile);
        fclose(outfile);
        return 1;
    }

    // Allocate memory for image buffers
    gray *image = (gray *)malloc(rows * cols * sizeof(gray));
    gray *blurred_image = (gray *)malloc(rows * cols * sizeof(gray));
    gray *grad_x = (gray *)malloc(rows * cols * sizeof(gray));
    gray *grad_y = (gray *)malloc(rows * cols * sizeof(gray));
    gray *magnitude = (gray *)malloc(rows * cols * sizeof(gray));
    gray *non_max = (gray *)malloc(rows * cols * sizeof(gray));
    gray *edges = (gray *)malloc(rows * cols * sizeof(gray));

    if (!image || !blurred_image || !grad_x || !grad_y || !magnitude || !non_max || !edges) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(infile);
        fclose(outfile);
        return 1;
    }

    // Load image data based on format
    if (strcmp(magic, "P5") == 0) {
        // Binary format (P5)
        if (fread(image, sizeof(gray), rows * cols, infile) != (size_t)(rows * cols)) {
            fprintf(stderr, "Error reading binary image data.\n");
            fclose(infile);
            fclose(outfile);
            free(image);
            free(blurred_image);
            free(grad_x);
            free(grad_y);
            free(magnitude);
            free(non_max);
            free(edges);
            return 1;
        }
    } else if (strcmp(magic, "P2") == 0) {
        // ASCII format (P2)
        for (int i = 0; i < rows * cols; i++) {
            int pixel_val;
            if (fscanf(infile, "%d", &pixel_val) != 1) {
                fprintf(stderr, "Error reading ASCII image data.\n");
                fclose(infile);
                fclose(outfile);
                free(image);
                free(blurred_image);
                free(grad_x);
                free(grad_y);
                free(magnitude);
                free(non_max);
                free(edges);
                return 1;
            }
            image[i] = (gray)pixel_val; // Store as grayscale value
        }
    } else {
        fprintf(stderr, "Error: Unsupported file format. This program only supports PGM (P2 or P5).\n");
        fclose(infile);
        fclose(outfile);
        free(image);
        free(blurred_image);
        free(grad_x);
        free(grad_y);
        free(magnitude);
        free(non_max);
        free(edges);
        return 1;
    }
    fclose(infile);

    // Apply image processing
    gaussian_filter(image, blurred_image, rows, cols);
    compute_gradient_x(blurred_image, grad_x, rows, cols);
    compute_gradient_y(blurred_image, grad_y, rows, cols);
    compute_gradient_magnitude(grad_x, grad_y, magnitude, rows, cols);
    non_maximum_suppression(magnitude, grad_x, grad_y, non_max, rows, cols);


    // normalize gradient magnitudes to range 0-255
    /* double max_magnitude = 0.0;
    for (i = 0; i < rows * cols; i++)
    {
        if (magnitude[i] > max_magnitude)
        {
            max_magnitude = magnitude[i];
        }
    }

    // scale all magnitudes
    for (i = 0; i < rows * cols; i++)
    {
        magnitude[i] = (gray)((magnitude[i] / max_magnitude) * 255);
    } */

    // Hysteresis thresholding
    int low_thresh = atoi(argv[3]);
    int high_thresh = atoi(argv[4]);
    hysteresis_thresholding(non_max, edges, rows, cols, low_thresh, high_thresh);

    // Write output as binary PGM (P5)
    fprintf(outfile, "P5\n%d %d\n%d\n", cols, rows, maxval);
    if (fwrite(edges, sizeof(gray), rows * cols, outfile) != (size_t)(rows * cols)) {
        fprintf(stderr, "Error writing image data to output file.\n");
    }
    fclose(outfile);

    // Free allocated memory
    free(image);
    free(blurred_image);
    free(grad_x);
    free(grad_y);
    free(magnitude);
    free(non_max);
    free(edges);

    return 0;
}
