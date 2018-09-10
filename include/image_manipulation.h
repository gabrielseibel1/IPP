/**
 * Declarations for image manipulation core library.
 * @author Gabriel de Souza Seibel
 * @date 05/09/2018
 */

#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

#ifndef FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H
#define FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H

struct error_manager {
    struct jpeg_error_mgr pub;  // "public" fields

    jmp_buf setjmp_buffer;  // for return to caller
};
typedef struct error_manager * error_manager_ptr;

int open(char *name);
int save_as(FILE* file, char *name);
int save(FILE* file);
int mirror_horizontally();
int mirror_vertically();
int to_grayscale();
int quantize_tones(int n_tones);

/**
 * Compresses an image buffer to a file using JPEG algorithm.
 * @param image_buffer 1D buffer with R,G,B,R,G,B ... info.
 * @param image_height the height of the image in pixels (number of rows).
 * @param image_width the width of the image in pixels (number of columns).
 * @param output_filename the name of the output file.
 * @return Non-negative if successful, negative if encountered errors.
 */
int jpeg_compress(JSAMPLE *image_buffer, int image_height, int image_width, char *output_filename);

/**
 * Decompresses a JPEG image into an image buffer using JPEG algorithm.
 * @param pixels_array_ptr pointer to 2D buffer with R,G,B,R,G,B ... info.
 * @param input_filename the name of the input file.
 * @return Non-negative if successful, negative if encountered errors.
 */
int jpeg_decompress(unsigned char ***pixels_array_ptr, int *image_height, int *image_width, int *channels, char *input_filename);

/**
 * Replaces the standard error_exit method:
 * @param cinfo Contains information of a compression or a decompression
 */
void my_error_exit(j_common_ptr cinfo);

/**
 * Converts 2D pixel array to 1D JSAMPLE array (R,G,B,R,G,B ...).
 * @param pixel_array The 2D pixel array to convert.
 * @param height The height of the image.
 * @param width The width of the image.
 * @param components The number of channels of the image.
 * @return Allocated and set JSAMPLE array.
 */
JSAMPLE *pixel_array_to_jsample_array(unsigned char **pixel_array, int height, int width, int components);

#endif //FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H
