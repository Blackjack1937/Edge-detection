#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

int main(int argc, char *argv[])
{
    FILE *ifp, *ofp;
    gray *graymap, *grad_x, *grad_y, *magnitude;
    int ich1, ich2, rows, cols, maxval = 255, pgmraw_in;
    int i, j;

    if (argc != 3)
    {
        printf("\nUsage: %s file_in file_out \n\n", argv[0]);
        exit(0);
    }
    ifp = fopen(argv[1], "r");

    if (ifp == NULL)
    {
        printf("error in opening file %s\n", argv[1]);
        exit(1);
    }

    // magic number
    ich1 = getc(ifp);
    if (ich1 == EOF)
    {
        pm_erreur("EOF / read error / magic number");
    }
    ich2 = getc(ifp);
    if (ich2 == EOF)
    {
        pm_erreur("EOF /read error / magic number");
    }

    if (ich1 != 'P' || (ich2 != '2' && ich2 != '5'))
    {
        pm_erreur("Invalid PGM format. Expected P2 (ASCII) or P5 (binary).");
    }

    // is image p5 or p2
    pgmraw_in = (ich2 == '5');

    cols = pm_getint(ifp);
    rows = pm_getint(ifp);
    maxval = pm_getint(ifp);

    graymap = (gray *)malloc(cols * rows * sizeof(gray));
    grad_x = (gray *)malloc(cols * rows * sizeof(gray));
    grad_y = (gray *)malloc(cols * rows * sizeof(gray));
    magnitude = (gray *)malloc(cols * rows * sizeof(gray));

    if (!graymap || !grad_x || !grad_y || !magnitude)
    {
        pm_erreur("Memory allocation error");
    }
    // read image data
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            if (pgmraw_in)
            {
                graymap[i * cols + j] = pm_getrawbyte(ifp);
            }
            else
            {
                graymap[i * cols + j] = pm_getint(ifp);
            }
        }
    }

    fclose(ifp);

    compute_gradient_x(graymap, grad_x, rows, cols);
    compute_gradient_y(graymap, grad_y, rows, cols);
    compute_gradient_magnitude(grad_x, grad_y, magnitude, rows, cols);

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

    ofp = fopen(argv[2], "w");
    if (ofp == NULL)
    {
        printf("error in opening file %s\n", argv[2]);
        exit(1);
    }

    // write results
    fprintf(ofp, "P2\n%d %d\n%d\n", cols, rows, maxval);

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            fprintf(ofp, "%d ", magnitude[i * cols + j]);
        }
        fprintf(ofp, "\n");
    }

    fclose(ofp);
    free(graymap);
    free(grad_x);
    free(grad_y);
    free(magnitude);

    return 0;
}
