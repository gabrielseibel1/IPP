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

#define HISTOGRAM_SIZE 256
#define FILTER_SIZE 3

enum result {
    COMPRESSION_SUCCESS,
    COMPRESSION_FAILURE,
    DECOMPRESSION_SUCCESS,
    DECOMPRESSION_FAILURE,
    FOPEN_FAILURE
};

struct error_manager {
    struct jpeg_error_mgr pub;  // "public" fields

    jmp_buf setjmp_buffer;  // for return to caller
};
typedef struct error_manager *error_manager_ptr;

typedef struct image_struct {
    char *filename;
    int height;
    int width;
    J_COLOR_SPACE colorspace;
    int channels;
    unsigned char **pixels;
    enum result last_operation;
} image_t;

/**
 * Initializes an image_t in heap memory.
 * @return Pointer to initialized image_t.
 */
image_t *new_image();

unsigned char **new_unsigned_char_matrix(int rows, int cols);

/**
 * Initializes a histogram (256-elements int vector) in heap memory.
 * @return Pointer to initialized histogram_t.
 */
int *new_histogram();

image_t *copy_image(image_t *original);

/**
 * Compresses an image buffer to a file using JPEG algorithm.
 * @param image_buffer 1D buffer with R,G,B,R,G,B ... info.
 * @param image_height the height of the image in pixels (number of rows).
 * @param image_width the width of the image in pixels (number of columns).
 * @param output_filename the name of the output file.
 * @return Non-negative if successful, negative if encountered errors.
 */
void jpeg_compress(image_t *image, char *output_filename);

/**
 * Decompresses a JPEG image into an image buffer using JPEG algorithm.
 * @param pixels_array_ptr pointer to 2D buffer with R,G,B,R,G,B ... info.
 * @param input_filename the name of the input file.
 * @return Non-negative if successful, negative if encountered errors.
 */
image_t *jpeg_decompress(char *input_filename);

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
JSAMPLE *pixel_array_to_jsample_array(image_t *image);

/**
 * Converts 2D pixel array to 1D unsigned char array (R,G,B,R,G,B ...).
 * @param pixel_array The 2D pixel array to convert.
 * @param height The height of the image.
 * @param width The width of the image.
 * @param components The number of channels of the image.
 * @return Allocated and set unsigned char array.
 */
unsigned char *pixel_array_to_unsigned_char_array(image_t *image);

void mirror_horizontally(image_t* image);

void mirror_vertically(image_t *image);

void free_pixels(image_t *image);

image_t *get_displayable(image_t *image);

void rgb_to_luminance(image_t *image);

void luminance_to_rgb(image_t *image);

void quantize(image_t *image, int n_tones);

int *compute_histogram(image_t *image);

int *compute_norm_cum_histogram(image_t *image);

/**
 * Build and returns 256x256 image representing the histogram
 */
image_t *histogram_plot(int *histogram);

/**
 * Brightness adjustment by adding bias term to each pixel component
 */
void add_bias(image_t *image, double bias);

/**
 * Contrast adjustment by multiplying gain term to each pixel component
 */
void multiply_gain(image_t *image, double bias);

/**
 * Takes the negative of the image by making every pixel component value' = 255 - value
 */
void negative(image_t *image);

/**
 * Attempts to produce optimal contrast by equalizing the histogram of the image
 */
void equalize_histogram(image_t *image);

/**
 * Make source's histogram match target's
 */
void match_histogram(image_t *source, image_t *target);

/**
 * Zoom out of image using a sliding window of size sx by sy, taking the mean of the channels as it slides
 * @param image the image to zoom out
 * @param sx number of pixels in sliding window in horizontal axis
 * @param sy number of pixels in sliding window in vertical axis
 */
void zoom_out(image_t *image, int sx, int sy);

/**
 * Applies 2x2 times zoom in
 * @param image the image to zoom in
 */
void zoom_in(image_t *image);

/**
 * Rotates every pixel by 90 degrees clock-wise
 * @param image the image to be rotated
 */
void rotate_90_degrees_clock_wise(image_t *image);


/**
 * Convolve image with FILTER_SIZE by FILTER_SIZE filter
 * @param image the image to be convolved
 * @param filter the filter to use in the convolution
 */
void convolve(image_t *image, float **filter, boolean clamp);


float **new_filter(int size);
float **gaussian_filter();
float **laplacian_filter();
float **high_pass_filter();
float **prewitt_hx_filter();
float **prewitt_hy_filter();
float **sobel_hx_filter();
float **sobel_hy_filter();

#endif //FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H
