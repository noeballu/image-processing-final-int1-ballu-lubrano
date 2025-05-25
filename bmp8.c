#include <stdlib.h>
#include <string.h>
#include "bmp8.h"


t_bmp8 *bmp8_loadImage(const char *filename) {
    FILE *image = fopen(filename, "rb");

    if (image == NULL) {
        printf("Error while opening the file, the file does not exist!\n");
        return NULL;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);

    unsigned int width = *(unsigned int*)&header[18];
    unsigned int height = *(unsigned int*)&header[22];
    unsigned short colorDepth = *(unsigned short*)&header[28];
    unsigned int dataSize = *(unsigned int*)&header[34];

    if (colorDepth != 8) {
        printf("Error while opening the file, the file is not 8 bits deep!\n");
        fclose(image);
        return NULL;
    }

    t_bmp8 *bmpImage = (t_bmp8 *)malloc(sizeof(t_bmp8));
    memcpy(bmpImage->header, header, 54);
    fread(bmpImage->colorTable, sizeof(unsigned char), 1024, image);

    bmpImage->width = width;
    bmpImage->height = height;
    bmpImage->colorDepth = colorDepth;
    bmpImage->dataSize = dataSize;

    bmpImage->data = (unsigned char *)malloc(dataSize);
    fseek(image, *(unsigned int*)&header[10], SEEK_SET);
    fread(bmpImage->data, sizeof(unsigned char), dataSize, image);

    fclose(image);
    printf("Image loaded successfully!\n\n");
    return bmpImage;
}


void bmp8_saveImage(const char * filename, t_bmp8 * img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error during the opening of the file, (gdb work)");
        return;
    }

    size_t header = fwrite(img->header, sizeof(unsigned char), 54, file);
    if (header != 54) {
        printf("Error occurred while writing the header");
        fclose(file);
        return;
    }

    size_t colorTable = fwrite(img->colorTable, sizeof(unsigned char), 1024, file);
    if (colorTable != 1024) {
        printf("Error during the writing of color table");
        fclose(file);
        return;
    }

    size_t written = fwrite(img->data, sizeof(unsigned char), img->dataSize, file);
    if (written != img->dataSize) {
        printf("Error during the writing of pixel data");
        fclose(file);
        return;
    }

    fclose(file);
}


void bmp8_free(t_bmp8 *img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}


void bmp8_printInfo(t_bmp8 *img) {
    printf("Image Information\n");
    printf("Width       : %u pixels\n", img->width);
    printf("Height      : %u pixels\n", img->height);
    printf("Color Depth : %u bits\n", img->colorDepth);
    printf("Data Size   : %u bytes\n", img->dataSize);
}


void bmp8_negative(t_bmp8 *img) {
    unsigned int total = img->width * img->height;
    for (unsigned int i = 0; i < total; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

//separated clamping logic
void bmp8_brightness(t_bmp8 *img, int value) {
    int total = img->width * img->height;
    for (int i = 0; i < total; i++) {
        int result = img->data[i] + value;
        if (result < 0) result = 0;
        else if (result > 255) result = 255;
        img->data[i] = (unsigned char)result;
    }
}

//condensed threshold logic
void bmp8_threshold(t_bmp8 *img, int threshold) {
    int total = img->width * img->height;
    for (int i = 0; i < total; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}

//kernel access logic and bounds check
void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize) {
    int half = kernelSize / 2;
    unsigned char *temp = (unsigned char *)malloc(img->width * img->height);

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            float sum = 0.0;

            for (int ky = -half; ky <= half; ky++) {
                for (int kx = -half; kx <= half; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (nx >= 0 && nx < (int)img->width && ny >= 0 && ny < (int)img->height) {
                        int pixelIndex = ny * img->width + nx;
                        int kernelIndexY = ky + half;
                        int kernelIndexX = kx + half;
                        sum += img->data[pixelIndex] * kernel[kernelIndexY][kernelIndexX];
                    }
                }
            }

            int finalValue = (int)sum;
            if (finalValue < 0) finalValue = 0;
            if (finalValue > 255) finalValue = 255;
            temp[y * img->width + x] = (unsigned char)finalValue;
        }
    }

    //copy result
    memcpy(img->data, temp, img->width * img->height);
    free(temp);
}
