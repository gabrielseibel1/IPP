#include <stdio.h>
#include <image_manipulation.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("%s <input file path> <output file path>\n", argv[0]);
    }

    // Decompress source image
    image_t *image = jpeg_decompress(argv[1]);
    if (image->last_operation != DECOMPRESSION_SUCCESS) {
        fprintf(stderr, "Decompression failed for file %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Print all pixels RGB value
    /*for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width; j += 3) {
            printf("(%d,%d,%d)\n", (int) pixels_array[i][j], (int) pixels_array[i][j + 1], (int) pixels_array[i][j + 2]);
        }
    }*/

    // Compress pixels_array into output image
    jpeg_compress(image, argv[2]);
}