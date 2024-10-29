typedef unsigned char bit;
typedef unsigned char gray;

char pm_getc(FILE* file);
bit pm_getbit(FILE* file);
unsigned char pm_getrawbyte(FILE* file);
int pm_getint( FILE* file);

void pm_erreur(char *);

void compute_gradient_x(gray *image, gray *grad_x, int rows, int cols);
void compute_gradient_y(gray *image, gray *grad_y, int rows, int cols);
void compute_gradient_magnitude(gray *grad_x, gray *grad_y, gray *magnitude, int rows, int cols);
void threshold_gradient_magnitude(gray *magnitude, gray *output, int rows, int cols, int threshold);
void gaussian_filter(gray *input, gray *output, int rows, int cols);
void non_maximum_suppression(gray *magnitude, gray *grad_x, gray *grad_y, gray *output, int rows, int cols);
void hysteresis_thresholding(gray *magnitude, gray *output, int rows, int cols, int tlow, int thigh);