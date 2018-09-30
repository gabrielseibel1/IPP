/**
 * Definitions for image manipulation core library.
 * @author Gabriel de Souza Seibel
 * @date 05/09/2018
 */

#include <image_manipulation.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>
#include <memory.h>

unsigned char closest_level(unsigned char value, int n_tones);

int pixels_in_histogram(const int *hist);

int *compute_norm_cum_histogram(image_t *image);

int get_closest(int candidate1, int candidate2, int goal);

int *compute_histogram_matching(const int *hist_cum_source, const int *hist_cum_target);

image_t *new_image() {
    return malloc(sizeof(image_t));
}

image_t *copy_image(image_t *original) {
    image_t *copy = new_image();
    if (original->filename) copy->filename = strdup(original->filename);
    copy->width = original->width;
    copy->height = original->height;
    copy->colorspace = original->colorspace;
    copy->channels = original->channels;
    copy->last_operation = original->last_operation;

    copy->pixels = new_unsigned_char_matrix(copy->height, copy->width * copy->channels);
    for (int row = 0; row < copy->height; ++row) {
        memcpy(copy->pixels[row], original->pixels[row], sizeof(unsigned char) * copy->width * copy->channels);
    }

    return copy;
}

int *new_histogram() {
    int *histogram = malloc(HISTOGRAM_SIZE * sizeof(int));
    for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
        histogram[i] = 0;
    }
    return histogram;
}

void jpeg_compress(image_t *image, char *output_filename) {
    struct jpeg_compress_struct cinfo;
    struct error_manager jerr;

    FILE *output_file;
    JSAMPLE *jsample_array = pixel_array_to_jsample_array(image);
    JSAMPROW row_pointer[1];
    int row_stride;

    // Open the output file before doing anything else, so that the setjmp() error recovery below can assume the file is open.
    if ((output_file = fopen(output_filename, "wb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", output_filename);
        image->last_operation = FOPEN_FAILURE;
        return;
    }

    // Set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    // Establish the setjmp return context for my_error_exit to use.
    if (setjmp(jerr.setjmp_buffer)) {
        // Here the JPEG code has signaled an error. Clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress((j_decompress_ptr) &cinfo);
        fclose(output_file);
        image->last_operation = COMPRESSION_FAILURE;
        return;
    }

    // Allocate and initialize a JPEG compression object
    jpeg_create_compress(&cinfo);

    // Specify the destination for the compressed data (eg, a file)
    jpeg_stdio_dest(&cinfo, output_file);

    // Set parameters for compression, including image size & colorspace
    cinfo.image_width = (JDIMENSION) image->width;
    cinfo.image_height = (JDIMENSION) image->height;
    cinfo.input_components = image->channels;
    cinfo.in_color_space = image->colorspace;
    jpeg_set_defaults(&cinfo);
    // TODO account for quantization with jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Compress each line to the output file
    row_stride = cinfo.image_width * cinfo.input_components;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &jsample_array[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);
    fclose(output_file);

    // Release the JPEG compression object
    jpeg_destroy_compress(&cinfo);

    image->last_operation = COMPRESSION_SUCCESS;
}

image_t *jpeg_decompress(char *input_filename) {
    image_t *image = new_image();

    struct jpeg_decompress_struct cinfo;
    struct error_manager jerr;

    FILE *input_file;
    JSAMPARRAY line_buffer;
    int row_stride;

    // Open the input file before doing anything else, so that the setjmp() error recovery below can assume the file is open.
    if ((input_file = fopen(input_filename, "rb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", input_filename);
        image->last_operation = FOPEN_FAILURE;
        return image;
    }

    // Set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    // Establish the setjmp return context for my_error_exit to use.
    if (setjmp(jerr.setjmp_buffer)) {
        // Here the JPEG code has signaled an error. Clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        fclose(input_file);
        image->last_operation = DECOMPRESSION_FAILURE;
        return image;
    }

    // Allocate and initialize a JPEG decompression object
    jpeg_create_decompress(&cinfo);

    // Specify the source of the compressed data (eg, a file)
    jpeg_stdio_src(&cinfo, input_file);

    // Read file parameters and image info with jpeg_read_header()
    (void) jpeg_read_header(&cinfo, TRUE);

    // Set parameters for decompression: do nothing since we want the defaults from jpeg_read_header()

    // Start decompression
    (void) jpeg_start_decompress(&cinfo);

    // Set fields with image info
    image->filename = strdup(input_filename);
    image->height = cinfo.output_height;
    image->width = cinfo.output_width;
    image->channels = cinfo.output_components;
    image->colorspace = cinfo.out_color_space;

    // Calculate physical/array size of a line
    row_stride = cinfo.output_width * cinfo.output_components;
    // Make a one-row-high sample array that will go away when done with image
    line_buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, (JDIMENSION) row_stride, 1);

    // Build 2D array to hold RGB (or luminance) values of all pixels
    image->pixels = new_unsigned_char_matrix(cinfo.output_height, row_stride);

    // Decompress each line from the input file to buffer and copy to 2D array
    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, line_buffer, 1);
        memcpy(image->pixels[cinfo.output_scanline - 1], line_buffer[0], row_stride * sizeof(unsigned char));
    }

    // Release JPEG decompression object
    jpeg_destroy_decompress(&cinfo);

    // Close input file
    fclose(input_file);

    image->last_operation = DECOMPRESSION_SUCCESS;
    return image;
}

unsigned char **new_unsigned_char_matrix(int rows, int cols) {
    unsigned char **pixel_array = (unsigned char **) malloc(rows * sizeof(unsigned char *));
    for (int i = 0; i < rows; ++i) {
        pixel_array[i] = (unsigned char *) malloc(cols * sizeof(unsigned char));
    }
    return pixel_array;
}

void my_error_exit(j_common_ptr cinfo) {
    // cinfo->err really points to a error_manager struct, so coerce pointer
    error_manager_ptr manager_ptr = (error_manager_ptr) cinfo->err;

    // Display error message
    (*cinfo->err->output_message)(cinfo);

    // Return control to the setjmp point
    longjmp(manager_ptr->setjmp_buffer, 1);
}

JSAMPLE *pixel_array_to_jsample_array(image_t *image) {
    JSAMPLE *jsample_array = (JSAMPLE *) malloc((size_t) image->height * image->width * image->channels);
    int jsample_index = 0;
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width * image->channels; ++j) {
            jsample_array[jsample_index++] = (JSAMPLE) image->pixels[i][j];
        }
    }
    return jsample_array;
}

unsigned char *pixel_array_to_unsigned_char_array(image_t *image) {
    unsigned char *array = (unsigned char *) malloc((size_t) image->height * image->width * image->channels);
    int index = 0;
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width * image->channels; ++j) {
            array[index++] = image->pixels[i][j];
        }
    }
    return array;
}

void mirror_vertically(image_t *image) {
    for (int top = 0, bot = image->height - 1; top < image->height / 2; ++top, --bot) {
        unsigned char *swap = image->pixels[top];
        image->pixels[top] = image->pixels[bot];
        image->pixels[bot] = swap;
    }
}

void mirror_horizontally(image_t *image) {
    // Iterate over lines
    for (int i = 0; i < image->height; ++i) {
        // Iterate over columns
        for (int left = 0, right = image->channels * (image->width - 1);
             left < (image->width * image->channels) / 2; left += image->channels, right -= image->channels) {

            // Swap all channels
            unsigned char *swap = malloc(sizeof(unsigned char) * image->channels);
            for (int c = 0; c < image->channels; ++c) {
                swap[c] = image->pixels[i][left + c];
                image->pixels[i][left + c] = image->pixels[i][right + c];
                image->pixels[i][right + c] = swap[c];
            }
        }
    }
}

void free_pixels(image_t *image) {
    for (int row = 0; row < image->height; ++row) {
        free(image->pixels[row]);
    }
    free(image->pixels);
}

image_t *get_displayable(image_t *image) {
    if (image->colorspace == JCS_RGB) return image;

    image_t *displayable = copy_image(image);
    luminance_to_rgb(displayable);

    return displayable;
}

void rgb_to_luminance(image_t *image) {
    if (image->colorspace == JCS_GRAYSCALE) {
        printf("Already grayscale!\n");
        return;
    }

    unsigned char **new_pixels = new_unsigned_char_matrix(image->height, image->width);
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width * image->channels; j += image->channels) {
            int luminance = (int) (0.299 * (int) image->pixels[i][j] +
                                   0.587 * (int) image->pixels[i][j + 1] +
                                   0.114 * (int) image->pixels[i][j + 2]);

            new_pixels[i][j / 3] = (unsigned char) luminance;
        }
    }

    image->colorspace = JCS_GRAYSCALE;
    image->channels = 1;

    free_pixels(image);
    image->pixels = new_pixels;
}

void luminance_to_rgb(image_t *image) {
    if (image->colorspace == JCS_RGB) return;

    unsigned char **new_pixels = new_unsigned_char_matrix(image->height, image->width * 3);
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width; ++j) {
            for (int c = 0; c < 3; ++c) {
                new_pixels[i][j * 3 + c] = image->pixels[i][j];
            }
        }
    }

    image->colorspace = JCS_RGB;
    image->channels = 3;

    free_pixels(image);
    image->pixels = new_pixels;
}

void quantize(image_t *image, int n_tones) {
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width * image->channels; j += image->channels) {
            for (int c = 0; c < image->channels; ++c) {
                image->pixels[i][j + c] = closest_level(image->pixels[i][j + c], n_tones);
            }
        }
    }
}

unsigned char closest_level(unsigned char value, int n_tones) {
    float step = (float) 255 / (n_tones - 1);
    float min = 0;
    while (!(value >= min && value <= min + step)) {
        min += step;
    }

    float max = (min + step >= 255) ? 255 : min + step;
    return (unsigned char) (abs((int) max - value) < abs((int) min - value) ? max : min);
}

int *compute_histogram(image_t *image) {
    int *histogram = new_histogram();

    image_t *gs_image = image;
    if (image->colorspace == JCS_RGB) {
        gs_image = copy_image(image);
        rgb_to_luminance(gs_image);
    }

    for (int row = 0; row < gs_image->height; ++row) {
        for (int col = 0; col < gs_image->width; ++col) {
            ++histogram[gs_image->pixels[row][col]];
        }
    }

    return histogram;
}

image_t *histogram_plot(int *histogram) {
    image_t *plot = new_image();
    plot->height = HISTOGRAM_SIZE;
    plot->width = HISTOGRAM_SIZE;
    plot->channels = 1;
    plot->colorspace = JCS_GRAYSCALE;
    plot->pixels = new_unsigned_char_matrix(plot->height, plot->width);

    int histogram_max_value = 0;
    for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
        if (histogram[i] > histogram_max_value) {
            histogram_max_value = histogram[i];
        }
    }

    int normalized_histogram[HISTOGRAM_SIZE];
    float scale_factor = (float) (HISTOGRAM_SIZE - 1) / histogram_max_value;
    for (int col = 0; col < HISTOGRAM_SIZE; ++col) {
        normalized_histogram[col] = (int) (scale_factor * histogram[col]);

        //paint column accordingly
        int bottom = plot->height - 1;
        for (int row = bottom; row > bottom - normalized_histogram[col]; --row) {
            plot->pixels[row][col] = 0; //black
        }
        for (int row = bottom - normalized_histogram[col]; row >= 0; --row) {
            plot->pixels[row][col] = 255; //white
        }
    }

    return plot;
}

void add_bias(image_t *image, double bias) {
    for (int h = 0; h < image->height; ++h) {
        for (int w = 0; w < image->width * image->channels; w += image->channels) {
            for (int c = 0; c < image->channels; ++c) {
                double sum = image->pixels[h][w + c] + bias;

                //verify saturation
                if (sum > 255) {
                    image->pixels[h][w + c] = 255;
                } else if (sum < 0) {
                    image->pixels[h][w + c] = 0;
                } else {
                    image->pixels[h][w + c] = (unsigned char) sum;
                }
            }
        }
    }
}

void multiply_gain(image_t *image, double gain) {
    for (int h = 0; h < image->height; ++h) {
        for (int w = 0; w < image->width * image->channels; w += image->channels) {
            for (int c = 0; c < image->channels; ++c) {
                double mult = image->pixels[h][w + c] * gain;

                //verify saturation
                if (mult > 255) {
                    image->pixels[h][w + c] = 255;
                } else {
                    image->pixels[h][w + c] = (unsigned char) mult;
                }
            }
        }
    }
}

void negative(image_t *image) {
    for (int h = 0; h < image->height; ++h) {
        for (int w = 0; w < image->width * image->channels; w += image->channels) {
            for (int c = 0; c < image->channels; ++c) {
                image->pixels[h][w + c] = (unsigned char) 255 - image->pixels[h][w + c];
            }
        }
    }
}

void equalize_histogram(image_t *image) {
    int *hist_cum = compute_norm_cum_histogram(image);

    for (int h = 0; h < image->height; ++h) {
        for (int w = 0; w < image->width * image->channels; w += image->channels) {
            for (int c = 0; c < image->channels; ++c) {
                image->pixels[h][w + c] = (unsigned char) hist_cum[image->pixels[h][w + c]];
            }
        }
    }
}

int *compute_norm_cum_histogram(image_t *image) {
    image_t *gs_image = image;
    if (image->colorspace == JCS_RGB) {
        gs_image = copy_image(image);
        rgb_to_luminance(gs_image);
    }

    int *hist = compute_histogram(gs_image);
    int *hist_cum = new_histogram();
    int pixels_in_hist = pixels_in_histogram(hist);

    double scale_factor = (double) 255 / pixels_in_hist;

    hist_cum[0] = (int) (scale_factor * hist[0]);
    for (int i = 1; i < HISTOGRAM_SIZE; ++i) {
        hist_cum[i] = (int) (hist_cum[i - 1] + scale_factor * hist[i]);
    }
    return hist_cum;
}

int pixels_in_histogram(const int *hist) {
    int pixels_in_hist = 0;
    for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
        pixels_in_hist += hist[i];
    }
    return pixels_in_hist;
}

/**
 * For each tone, find the tone that has the closest number of corresponding pixels
 * @param tone the tone to have its pixel count mapped
 * @param hist_cum_source source cumulative histogram
 * @param hist_cum_target target cumulative histogram
 * @return the tone that has the closest pixel count in target histogram
 */
int find_target_tone_closest_to(int tone, const int *hist_cum_source, const int *hist_cum_target) {
    int ideal_pixel_count = hist_cum_source[tone];
    int tone_for_closest_pixel_count = 0;

    for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
        if (abs(ideal_pixel_count - hist_cum_target[i]) <
            abs(ideal_pixel_count - hist_cum_target[tone_for_closest_pixel_count])) {
            tone_for_closest_pixel_count = i;
        }
    }

    return tone_for_closest_pixel_count;
}

/**
 * Builds a histogram that makes that makes source match target
 * @param hist_cum_source source cumulative histogram
 * @param hist_cum_target target cumulative histogram
 * @return
 */
int *compute_histogram_matching(const int *hist_cum_source, const int *hist_cum_target) {
    int *histogram_matching = new_histogram();
    for (int tone = 0; tone < HISTOGRAM_SIZE; ++tone) {
        histogram_matching[tone] = find_target_tone_closest_to(tone, hist_cum_source, hist_cum_target);
    }
    return histogram_matching;
}

void match_histogram(image_t *source, image_t *target) {
    //assert images are in grayscale
    rgb_to_luminance(source);
    rgb_to_luminance(target);

    int *hist_cum_source = compute_norm_cum_histogram(source);
    int *hist_cum_target = compute_norm_cum_histogram(target);

    int *histogram_matching = compute_histogram_matching(hist_cum_source, hist_cum_target);

    for (int h = 0; h < source->height; ++h) {
        for (int w = 0; w < source->width; ++w) {
            source->pixels[h][w] = (unsigned char) histogram_matching[source->pixels[h][w]];
        }
    }
}
