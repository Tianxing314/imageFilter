//
//  filter.c
//  filter
//
//  Created by Tianxing Li on 2019-11-26.
//  Copyright © 2019 Tianxing Li. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//writePPM and readPPM refer to SFWR2S03 Fall 2017 Assignment4 solutions
typedef struct {
    int r;
    int g;
    int b;
} RGB;

void writePPM(const char* file, int width, int height, int max,
    const RGB* image)
{
    int i;

    // open file for writing
    FILE* fd;
    fd = fopen(file, "w");

    // output the header
    fprintf(fd, "P3\n");
    fprintf(fd, "%d %d\n%d\n", width, height, max);

    for (i = 0; i < height * width; i++) {
        const RGB* p = image + i;
        fprintf(fd, "%d %d %d ", p->r, p->g, p->b);
    }
    fclose(fd);
}

RGB* readPPM(const char* file, int* width, int* height, int* max)
{
    /* Read a PPM P3 image from a file into a buffer.  Return the
     buffer, width, height, and the max value in the image. */

    FILE* fd;
    char c;
    int i, n;
    char b[100];
    int red, green, blue;

    // check if P3 ppm format
    fd = fopen(file, "r");
    n = fscanf(fd, "%[^\n] ", b);
    if (b[0] != 'P' || b[1] != '3') {
        printf("%s is not a PPM file!\n", file);
        exit(0);
    }
    
    n = fscanf(fd, "%c", &c);
    while (c == '#') {
        n = fscanf(fd, "%[^\n] ", b);
        printf("%s\n", b);
        n = fscanf(fd, "%c", &c);
    }
    ungetc(c, fd);
    n = fscanf(fd, "%d %d %d", width, height, max);
    assert(n == 3);

    // size of image
    int size = *width * (*height);

    RGB* image = (RGB*)malloc(size * sizeof(RGB));
    assert(image);

    for (i = 0; i < size; i++) {
        n = fscanf(fd, "%d %d %d", &red, &green, &blue);
        assert(n == 3);
        image[i].r = red;
        image[i].g = green;
        image[i].b = blue;
    }

    fclose(fd);
    return image;
}

int* read_kernel(const char* file, int* n, int* scale)
{

    FILE* fd;
    int check;
    int element;

    fd = fopen(file, "r");
    
    check = fscanf(fd, "%d", n); //read value of n
    
    check = fscanf(fd, "%d", scale); //read value of scale
    
    int size = (*n) * (*n);

    int* kernel = (int*)malloc(size * sizeof(int));
    assert(kernel);

    int i;
    for (i = 0; i < size; i++) {
        check = fscanf(fd, "%d", &element);
        assert(check == 1);
        kernel[i] = element;
    }

    fclose(fd);
    return kernel;
}



int* reverse(int* arr, int len) {
    int* res = malloc(len*sizeof(int));
    int i;
    for (i=0; i<len; i++) {
        res[i] = arr[len-i-1];
    }
    return res;
}

int convolute(int scale, int n, int* kernel, int* partial_img) {
    int accumulator = 0;
    int len = n * n;
    int* rev_kernel = reverse(kernel, len);
    int i;
    for (i=0; i<len; i++) {
        accumulator += rev_kernel[i] * partial_img[i];
    }
    return accumulator / scale;
}

int* create_partial_img(int* img, int n, int i, int j, int w, int h) {
    int* arr = malloc((n * n) * sizeof(int));
    int index = 0;
    int row1 = i - n/2; //row = [i-n/2, i+n/2]
    int row2 = i + n/2; //row = [i-n/2, i+n/2]
    int col1 = j - n/2; //col = [j-n/2, j+n/2]
    int col2 = j + n/2; //col = [j-n/2, j+n/2]
    int m;
    for (m=row1; m<=row2; m++) {
        int n;
        for (n=col1; n<=col2; n++){
            if (n<0 || n>w-1 || m<0 || m>h-1) {
                arr[index++] = 0;
                continue;
            }
            arr[index++] = img[m*w+n];
        }
    }
    return arr;
}

int* filter_pixel(int scale, int n, int w, int h, int* kernel, int* pixel_image) {
    int* res = malloc((w * h) * sizeof(int));
    int i, j;
    for (i=0; i<h; i++) {
        for (j=0; j<w; j++) {
            int* partial_img = create_partial_img(pixel_image, n, i, j, w, h);
            res[i*w+j] = convolute(scale, n, kernel, partial_img);
        }
    }
    return res;
}

RGB* filter_image(int scale, int n, int* kernel, int w, int h, RGB* image) {
    int len = w * h;
    int* red_img = malloc(len * sizeof(int));
    int* green_img = malloc(len * sizeof(int));
    int* blue_img = malloc(len * sizeof(int));
    int i;
    for (i=0; i<len; i++) {
        red_img[i] = image[i].r;
        green_img[i] = image[i].g;
        blue_img[i] = image[i].b;
    }
    int* red_filtered = filter_pixel(scale, n, w, h, kernel, red_img);
    int* green_filtered = filter_pixel(scale, n, w, h, kernel, green_img);
    int* blue_filtered = filter_pixel(scale, n, w, h, kernel, blue_img);
    
    RGB* res = malloc(len * sizeof(RGB));
    for (i=0; i<len; i++) {
        RGB rgb = {
            .r = red_filtered[i],
            .g = green_filtered[i],
            .b = blue_filtered[i]
        };
        res[i] = rgb;
    }
    free(red_img);
    free(green_img);
    free(blue_img);
    free(red_filtered);
    free(green_filtered);
    free(blue_filtered);
    return res;
}

int main(int argc, const char** argv) {
    if (argc != 4) {
        printf("Usage: ./filter input.ppm kernel output.ppm\n");
        return -1;
    }
    const char *input_file, *kernel_file, *output_file;
    input_file = argv[1];
    kernel_file = argv[2];
    output_file = argv[3];

    int width;
    int height;
    int max;
    int n;
    int scale;
    int* kernel = read_kernel(kernel_file, &n, &scale);

    RGB* image = readPPM(input_file, &width, &height, &max);
    RGB* image_filtered = filter_image(scale, n, kernel, width, height, image);
    writePPM(output_file, width, height, max, image_filtered);
    free(kernel);
    free(image);
    free(image_filtered);;
    return 0;
}
