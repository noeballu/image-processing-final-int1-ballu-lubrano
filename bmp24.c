#include "bmp24.h"
#include <math.h>

int clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

// Memory allocation

t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = malloc(height * sizeof(t_pixel *));
    if (!pixels) return NULL;

    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(t_pixel));
        if (!pixels[i]) return NULL;
    }
    return pixels;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (!img) return NULL;
    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
    img->data = bmp24_allocateDataPixels(width, height);
    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (!img) return;
    bmp24_freeDataPixels(img->data, img->height);
    free(img);
}

// Raw read/write

void file_rawRead(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

void file_rawWrite(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

// Load & save

t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Erreur ouverture fichier %s\n", filename);
        return NULL;
    }

    // Manually read only the essential header fields
    uint16_t type;
    int32_t width, height;
    uint16_t bits;
    uint32_t compression, offset;

    fseek(f, 0, SEEK_SET);
    // BMP signature
    fread(&type, sizeof(uint16_t), 1, f);

    fseek(f, 18, SEEK_SET);
    fread(&width, sizeof(int32_t), 1, f);
    fread(&height, sizeof(int32_t), 1, f);

    fseek(f, 28, SEEK_SET);
    fread(&bits, sizeof(uint16_t), 1, f);

    fseek(f, 30, SEEK_SET);
    fread(&compression, sizeof(uint32_t), 1, f);

    fseek(f, 10, SEEK_SET);
    fread(&offset, sizeof(uint32_t), 1, f);

    // Validate BMP 24-bit uncompressed format
    if (type != 0x4D42 || bits != 24 || compression != 0) {
        printf("Incompatible file. BMP 24 bits must be uncompressed .\n");
        fclose(f);
        return NULL;
    }

    // Allocate the structure
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    img->width = width;
    img->height = height;
    img->colorDepth = bits;
    img->data = bmp24_allocateDataPixels(width, height);
    if (!img->data) {
        fclose(f);
        free(img);
        return NULL;
    }

    // // Read pixel data starting at "offset", line by line (a big issues in the process of the bm24.c
    fseek(f, offset, SEEK_SET);
    int padding = (4 - (width * 3) % 4) % 4;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char bgr[3];
            fread(bgr, 1, 3, f);
            img->data[height - 1 - y][x].blue = bgr[0];
            img->data[height - 1 - y][x].green = bgr[1];
            img->data[height - 1 - y][x].red = bgr[2];
        }
        // skip padding bytes
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    printf("Image loaded : %dx%d\n", width, height);
    return img;
}

void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Writing error  %s\n", filename);
        return;
    }

    // // Write BMP header (14 bytes)
    uint16_t type = 0x4D42;
    uint32_t offset = 54;
    uint32_t size = offset + (img->width * 3 + (4 - (img->width * 3 % 4)) % 4) * img->height;
    uint16_t reserved = 0;

    // Write BMP info header (40 bytes)
    fwrite(&type, sizeof(uint16_t), 1, f);
    fwrite(&size, sizeof(uint32_t), 1, f);
    fwrite(&reserved, sizeof(uint16_t), 1, f);
    fwrite(&reserved, sizeof(uint16_t), 1, f);
    fwrite(&offset, sizeof(uint32_t), 1, f);

    // Write header info (40 octets)
    uint32_t headerSize = 40;
    uint16_t planes = 1;
    uint16_t bits = 24;
    uint32_t compression = 0;
    uint32_t imageSize = size - offset;
    int32_t resolution = 2835;

    fwrite(&headerSize, sizeof(uint32_t), 1, f);
    fwrite(&img->width, sizeof(int32_t), 1, f);
    fwrite(&img->height, sizeof(int32_t), 1, f);
    fwrite(&planes, sizeof(uint16_t), 1, f);
    fwrite(&bits, sizeof(uint16_t), 1, f);
    fwrite(&compression, sizeof(uint32_t), 1, f);
    fwrite(&imageSize, sizeof(uint32_t), 1, f);
    fwrite(&resolution, sizeof(int32_t), 1, f);
    fwrite(&resolution, sizeof(int32_t), 1, f);
    // ncolors = 0
    fwrite(&compression, sizeof(uint32_t), 1, f);
    // important colors = 0
    fwrite(&compression, sizeof(uint32_t), 1, f);

    // Write pixels
    int padding = (4 - (img->width * 3) % 4) % 4;
    unsigned char pad[3] = {0, 0, 0};

    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            unsigned char bgr[3] = {
                    img->data[y][x].blue,
                    img->data[y][x].green,
                    img->data[y][x].red
            };
            fwrite(bgr, 1, 3, f);
        }
        fwrite(pad, 1, padding, f);
    }

    fclose(f);
    printf("Image save successfully in %s\n", filename);
}

// Pixel I/O

void bmp24_readPixelValue(t_bmp24 *image, int x, int y, FILE *file) {
    fseek(file, image->header.offset + ((image->height - 1 - y) * ((image->width * 3 + 3) & ~3)) + x * 3, SEEK_SET);
    fread(&image->data[y][x].blue, 1, 1, file);
    fread(&image->data[y][x].green, 1, 1, file);
    fread(&image->data[y][x].red, 1, 1, file);
}

void bmp24_readPixelData(t_bmp24 *image, FILE *file) {
    int rowSize = ((image->width * 3 + 3) / 4) * 4;
    uint8_t *row = malloc(rowSize);

    for (int y = image->height - 1; y >= 0; y--) {
        fread(row, 1, rowSize, file);
        for (int x = 0; x < image->width; x++) {
            image->data[y][x].blue  = row[x * 3 + 0];
            image->data[y][x].green = row[x * 3 + 1];
            image->data[y][x].red   = row[x * 3 + 2];
        }
    }
    free(row);
}

void bmp24_writePixelValue(t_bmp24 *img, int x, int y, FILE *file) {
    uint8_t bgr[3] = {
            img->data[y][x].blue,
            img->data[y][x].green,
            img->data[y][x].red
    };
    fwrite(bgr, 1, 3, file);
}

void bmp24_writePixelData(t_bmp24 *img, FILE *file) {
    int rowSize = ((img->width * 3 + 3) / 4) * 4;
    int padding = rowSize - img->width * 3;
    uint8_t pad[3] = {0};

    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            bmp24_writePixelValue(img, x, y, file);
        }
        fwrite(pad, 1, padding, file);
    }
}

// Filters

void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel *p = &img->data[y][x];
            p->red = 255 - p->red;
            p->green = 255 - p->green;
            p->blue = 255 - p->blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++)
        for (int x = 0; x < img->width; x++) {
            uint8_t gray = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            img->data[y][x].red = img->data[y][x].green = img->data[y][x].blue = gray;
        }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel *p = &img->data[y][x];
            p->red = fminf(fmaxf(p->red + value, 0), 255);
            p->green = fminf(fmaxf(p->green + value, 0), 255);
            p->blue = fminf(fmaxf(p->blue + value, 0), 255);
        }
    }
}


t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize) {
    int n = kernelSize / 2;
    float r = 0, g = 0, b = 0;

    for (int ky = -n; ky <= n; ky++) {
        for (int kx = -n; kx <= n; kx++) {
            int px = x + kx;
            int py = y + ky;
            if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                t_pixel p = img->data[py][px];
                float coeff = kernel[ky + n][kx + n];
                r += p.red * coeff;
                g += p.green * coeff;
                b += p.blue * coeff;
            }
        }
    }

    t_pixel result = {
            .red   = clamp(round(r)),
            .green = clamp(round(g)),
            .blue  = clamp(round(b))
    };
    return result;
}

void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    t_pixel **copy = bmp24_allocateDataPixels(img->width, img->height);
    for (int y = 0; y < img->height; y++)
        for (int x = 0; x < img->width; x++)
            copy[y][x] = bmp24_convolution(img, x, y, kernel, kernelSize);

    for (int y = 0; y < img->height; y++)
        for (int x = 0; x < img->width; x++)
            img->data[y][x] = copy[y][x];

    bmp24_freeDataPixels(copy, img->height);
}

void bmp24_boxBlur(t_bmp24 *img) {
    float box[3][3] = {
            {1/9.f, 1/9.f, 1/9.f},
            {1/9.f, 1/9.f, 1/9.f},
            {1/9.f, 1/9.f, 1/9.f}
    };
    float* kernel[3] = { box[0], box[1], box[2] };
    bmp24_applyFilter(img, kernel, 3);
}

void bmp24_gaussianBlur(t_bmp24 *img) {
    float gauss[3][3] = {
            {1/16.f, 2/16.f, 1/16.f},
            {2/16.f, 4/16.f, 2/16.f},
            {1/16.f, 2/16.f, 1/16.f}
    };
    float* kernel[3] = { gauss[0], gauss[1], gauss[2] };
    bmp24_applyFilter(img, kernel, 3);
}

void bmp24_outline(t_bmp24 *img) {
    float o[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = o[i][j];
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}

void bmp24_emboss(t_bmp24 *img) {
    float e[3][3] = {{-2, -1, 0}, {-1, 1, 1}, {0, 1, 2}};
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = e[i][j];
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}

void bmp24_sharpen(t_bmp24 *img) {
    float sharpen[3][3] = {
            { 0, -1,  0},
            {-1,  5, -1},
            { 0, -1,  0}
    };
    float* kernel[3] = { sharpen[0], sharpen[1], sharpen[2] };
    bmp24_applyFilter(img, kernel, 3);
}
