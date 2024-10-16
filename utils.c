#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>

char pm_getc(FILE *file)
{
    int ich;
    char ch;

    ich = getc(file);
    if (ich == EOF)
        pm_erreur("EOF / read error");
    ch = (char)ich;

    if (ch == '#')
    {
        do
        {
            ich = getc(file);
            if (ich == EOF)
                pm_erreur("EOF / read error");
            ch = (char)ich;
        } while (ch != '\n' && ch != '\r');
    }

    return ch;
}

bit pm_getbit(FILE *file)
{
    char ch;

    do
    {
        ch = pm_getc(file);
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

    if (ch != '0' && ch != '1')
        pm_erreur("read error : != [0|1]");

    return (ch == '1') ? 1 : 0;
}

unsigned char pm_getrawbyte(FILE *file)
{
    int iby;

    iby = getc(file);
    if (iby == EOF)
        pm_erreur("EOF / read error ");
    return (unsigned char)iby;
}

int pm_getint(FILE *file)
{
    char ch;
    int i;

    do
    {
        ch = pm_getc(file);
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

    if (ch < '0' || ch > '9')
        pm_erreur("read error : != [0-9]");

    i = 0;
    do
    {
        i = i * 10 + ch - '0';
        ch = pm_getc(file);
    } while (ch >= '0' && ch <= '9');

    return i;
}

void pm_erreur(char *texte)
{
    fprintf(stderr, "\n%s \n\n", texte);
    exit(1);
}

void compute_gradient_x(gray *image, gray *grad_x, int rows, int cols)
{
    int scharr_x[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};

    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            int sum = 0;
            for (int u = -1; u <= 1; u++)
            {
                for (int v = -1; v <= 1; v++)
                {
                    sum += image[(i + u) * cols + (j + v)] * scharr_x[u + 1][v + 1];
                }
            }
            grad_x[i * cols + j] = (gray)abs(sum); // store absolute gradient
        }
    }
}

void compute_gradient_y(gray *image, gray *grad_y, int rows, int cols)
{
    int scharr_y[3][3] = {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}};

    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            int sum = 0;
            for (int u = -1; u <= 1; u++)
            {
                for (int v = -1; v <= 1; v++)
                {
                    sum += image[(i + u) * cols + (j + v)] * scharr_y[u + 1][v + 1];
                }
            }
            grad_y[i * cols + j] = (gray)abs(sum);
        }
    }
}

void compute_gradient_magnitude(gray *grad_x, gray *grad_y, gray *magnitude, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int index = i * cols + j;
            magnitude[index] = (gray)sqrt(pow(grad_x[index], 2) + pow(grad_y[index], 2));
        }
    }
}
