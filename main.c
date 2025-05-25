#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp8.h"
#include "bmp24.h"

void menu8bit() {
    t_bmp8 *img = NULL;
    char filename[256];
    int choice, value;

    do {
        printf("\n8-bit Grayscale Menu \n");
        printf("1. Load Image\n");
        printf("2. Negative Filter\n");
        printf("3. Brightness Adjustment\n");
        printf("4. Thresholding\n");
        printf("5. Save Image\n");
        printf("6. Image Info\n");
        printf("7. Back to Main Menu\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = 0;
        }

        switch (choice) {
            case 1:
                printf("Enter BMP filename: ");
                scanf("%255s", filename);
                img = bmp8_loadImage(filename);
                break;
            case 2:
                if (img) bmp8_negative(img);
                else printf("Load an image first.\n");
                break;
            case 3:
                if (img) {
                    printf("Brightness value (-255 to 255): ");
                    scanf("%d", &value);
                    bmp8_brightness(img, value);
                } else printf("Load an image first.\n");
                break;
            case 4:
                if (img) {
                    printf("Threshold (0-255): ");
                    scanf("%d", &value);
                    bmp8_threshold(img, value);
                } else printf("Load an image first.\n");
                break;
            case 5:
                if (img) {
                    printf("Enter filename to save: ");
                    scanf("%255s", filename);
                    bmp8_saveImage(filename, img);
                } else printf("Load an image first.\n");
                break;
            case 6:
                if (img) bmp8_printInfo(img);
                else printf("Load an image first.\n");
                break;
            case 7:
                bmp8_free(img);
                img = NULL;
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 7);
}

void menu24bit() {
    t_bmp24 *img = NULL;
    char filename[256];
    int choice, value;

    do {
        printf("\n=== 24-bit Color Menu ===\n");
        printf("1. Load Image\n");
        printf("2. Negative Filter\n");
        printf("3. Brightness Adjustment\n");
        printf("4. Grayscale Conversion\n");
        printf("5. Apply Filter (Box Blur)\n");
        printf("6. Save Image\n");
        printf("7. Back to Main Menu\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = 0;
        }

        switch (choice) {
            case 1:
                printf("Enter BMP filename: ");
                scanf("%255s", filename);
                img = bmp24_loadImage(filename);
                break;
            case 2:
                if (img) bmp24_negative(img);
                else printf("Load an image first.\n");
                break;
            case 3:
                if (img) {
                    printf("Brightness value (-255 to 255): ");
                    scanf("%d", &value);
                    bmp24_brightness(img, value);
                } else printf("Load an image first.\n");
                break;
            case 4:
                if (img) bmp24_grayscale(img);
                else printf("Load an image first.\n");
                break;
            case 5:
                if (img) {
                    bmp24_boxBlur(img);
                    printf("Box blur filter applied.\n");
                } else printf("Load an image first.\n");
                break;
            case 6:
                if (img) {
                    printf("Enter filename to save: ");
                    scanf("%255s", filename);
                    bmp24_saveImage(img, filename);
                } else printf("Load an image first.\n");
                break;
            case 7:
                bmp24_free(img);
                img = NULL;
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 7);
}

int main() {
    int mode;

    do {
        printf("\nBMP Image Processor\n");
        printf("1. 8-bit Grayscale Mode\n");
        printf("2. 24-bit Color Mode\n");
        printf("3. Exit\n");
        printf("Select mode: ");
        if (scanf("%d", &mode) != 1) {
            while (getchar() != '\n');
            mode = 0;
        }

        switch (mode) {
            case 1:
                menu8bit();
                break;
            case 2:
                menu24bit();
                break;
            case 3:
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid option.\n");
        }
    } while (mode != 3);

    return 0;
}
