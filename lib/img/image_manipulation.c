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

int jpeg_compress(JSAMPLE *image_buffer, int image_height, int image_width, char *output_filename) {
    struct jpeg_compress_struct cinfo;
    struct error_manager jerr;

    FILE *output_file;
    JSAMPROW row_pointer[1];
    int row_stride;

    // Open the output file before doing anything else, so that the setjmp() error recovery below can assume the file is open.
    if ((output_file = fopen(output_filename, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", output_filename);
        exit(EXIT_FAILURE);
    }

    // Set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    // Establish the setjmp return context for my_error_exit to use.
    if (setjmp(jerr.setjmp_buffer)) {
        // Here the JPEG code has signaled an error. Clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        fclose(output_file);
        return 0;
    }

    // Allocate and initialize a JPEG compression object
    jpeg_create_compress(&cinfo);

    // Specify the destination for the compressed data (eg, a file)
    jpeg_stdio_dest(&cinfo, output_file);

    // Set parameters for compression, including image size & colorspace
    cinfo.image_width = (JDIMENSION) image_width;
    cinfo.image_height = (JDIMENSION) image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    // TODO account for quantization with jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Compress each line to the output file
    row_stride = image_width * cinfo.input_components;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);
    fclose(output_file);

    // Release the JPEG compression object
    jpeg_destroy_compress(&cinfo);

    return 0;
}

void my_error_exit(j_common_ptr cinfo) {
    // cinfo->err really points to a error_manager struct, so coerce pointer
    error_manager_ptr manager_ptr = (error_manager_ptr) cinfo->err;

    // Display error message
    (*cinfo->err->output_message)(cinfo);

    // Return control to the setjmp point
    longjmp(manager_ptr->setjmp_buffer, 1);
}

int jpeg_decompress(unsigned char ***pixels_array_ptr, int *image_height, int *image_width, int *channels, char *input_filename) {
    struct jpeg_decompress_struct cinfo;
    struct error_manager jerr;

    FILE *input_file;
    JSAMPARRAY line_buffer;
    int row_stride;

    // Open the input file before doing anything else, so that the setjmp() error recovery below can assume the file is open.
    if ((input_file = fopen(input_filename, "rb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", input_filename);
        return 0;
    }

    // Set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    // Establish the setjmp return context for my_error_exit to use.
    if (setjmp(jerr.setjmp_buffer)) {
        // Here the JPEG code has signaled an error. Clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        fclose(input_file);
        return 0;
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

    // Fill parameters with image info
    *image_height = cinfo.output_height;
    *image_width = cinfo.output_width;
    *channels = cinfo.output_components;

    // Calculate physical/array size of a line
    row_stride = cinfo.output_width * cinfo.output_components;
    // Make a one-row-high sample array that will go away when done with image
    line_buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, (JDIMENSION) row_stride, 1);

    // Build 2D array to hold RGB values of all pixels
    *pixels_array_ptr = (unsigned char **) malloc(cinfo.output_height * sizeof(unsigned char *));
    for (int i = 0; i < cinfo.output_height; ++i) {
        (*pixels_array_ptr)[i] = (unsigned char *) malloc(row_stride * sizeof(unsigned char));
    }

    // Decompress each line from the input file to buffer and copy to 2D array
    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, line_buffer, 1);
        memcpy((*pixels_array_ptr)[cinfo.output_scanline - 1], line_buffer[0], row_stride * sizeof(unsigned char));
    }

    // Release JPEG decompression object
    jpeg_destroy_decompress(&cinfo);

    // Close input file
    fclose(input_file);

    return 0;
}

JSAMPLE *pixel_array_to_jsample_array(unsigned char **pixel_array, int height, int width, int components) {
    JSAMPLE *jsample_array = (JSAMPLE *) malloc((size_t) height * width * components);
    int jsample_index = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width * components; ++j) {
            jsample_array[jsample_index++] = pixel_array[i][j];
        }
    }
    return jsample_array;
}
