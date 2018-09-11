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

    mirror_horizontally(image);

    // Compress pixels_array into output image
    jpeg_compress(image, argv[2]);
}