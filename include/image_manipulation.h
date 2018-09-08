/**
 * Declarations for image manipulation core library.
 * @author Gabriel de Souza Seibel
 * @date 05/09/2018
 */

#include <stdio.h>

#ifndef FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H
#define FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H

int save_as(FILE* file, char *name);
int save(FILE* file);
int mirror_horizontally();
int mirror_vertically();
int to_grayscale();
int quantize_tones(int n_tones);

#endif //FPI_ASSIGNMENT_1_IMAGE_MANIPULATION_H
