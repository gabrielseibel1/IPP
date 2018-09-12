/**
 * Definitions for image manipulation core library.
 * @author Gabriel de Souza Seibel
 * @date 05/09/2018
 */

#include <image_manipulation.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <memory.h>

image_t *new_image() {
    return malloc(sizeof(image_t));
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
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
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

    // Calculate physical/array size of a line
    row_stride = cinfo.output_width * cinfo.output_components;
    // Make a one-row-high sample array that will go away when done with image
    line_buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, (JDIMENSION) row_stride, 1);

    // Build 2D array to hold RGB values of all pixels
    image->pixel_array = (unsigned char **) malloc(cinfo.output_height * sizeof(unsigned char *));
    for (int i = 0; i < cinfo.output_height; ++i) {
        image->pixel_array[i] = (unsigned char *) malloc(row_stride * sizeof(unsigned char));
    }

    // Decompress each line from the input file to buffer and copy to 2D array
    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, line_buffer, 1);
        memcpy(image->pixel_array[cinfo.output_scanline - 1], line_buffer[0], row_stride * sizeof(unsigned char));
    }

    // Release JPEG decompression object
    jpeg_destroy_decompress(&cinfo);

    // Close input file
    fclose(input_file);

    image->last_operation = DECOMPRESSION_SUCCESS;
    return image;
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
            jsample_array[jsample_index++] = image->pixel_array[i][j];
        }
    }
    return jsample_array;
}

unsigned char *pixel_array_to_unsigned_char_array(image_t *image) {
    unsigned char *array = (unsigned char *) malloc((size_t) image->height * image->width * image->channels);
    int index = 0;
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width * image->channels; ++j) {
            array[index++] = image->pixel_array[i][j];
        }
    }
    return array;
}

void mirror_vertically(image_t *image) {
    for (int top = 0, bot = image->height - 1; top < image->height / 2; ++top, --bot) {
        unsigned char *swap = image->pixel_array[top];
        image->pixel_array[top] = image->pixel_array[bot];
        image->pixel_array[bot] = swap;
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
                swap[c] = image->pixel_array[i][left + c];
                image->pixel_array[i][left + c] = image->pixel_array[i][right + c];
                image->pixel_array[i][right + c] = swap[c];
            }
        }
    }
}

void to_gray_scale(image_t *image) {
    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width * image->channels; j += 3) {
            int luminance = (int) (0.299 * (int) image->pixel_array[i][j] +
                                   0.587 * (int) image->pixel_array[i][j + 1] +
                                   0.114 * (int) image->pixel_array[i][j + 2]);

            image->pixel_array[i][j] = (unsigned char) luminance;
            image->pixel_array[i][j + 1] = (unsigned char) luminance;
            image->pixel_array[i][j + 2] = (unsigned char) luminance;
        }
    }
}




