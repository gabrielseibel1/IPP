#include <stdio.h>
#include <image_manipulation.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("%s <input file path> <output file path>\n", argv[0]);
    }

    // Decompress source image
    unsigned char **pixels_array;
    int height, width, channels;
    jpeg_decompress(&pixels_array, &height, &width, &channels, argv[1]);

    // Print all pixels RGB value
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; j += 3) {
            //printf("(%d,%d,%d)\n", (int) pixels_array[i][j], (int) pixels_array[i][j + 1], (int) pixels_array[i][j + 2]);
        }
    }

    // Compress pixels_array into output image
    JSAMPLE *jsample_array = pixel_array_to_jsample_array(pixels_array, height, width, channels);
    jpeg_compress(jsample_array, height, width, argv[2]);
}