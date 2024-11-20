#ifndef __FILTROS_H
#define __FILTROS_H


#include "bmp.h"
#include <stdio.h>
#include <math.h>
#include <omp.h>  
#include <stdlib.h>
#include <string.h>

//Escala de grises
void grayscale(int height, int width, RGBTRIPLE image[height][width]);
//Filtro sepia
void sepia(int height, int width, RGBTRIPLE image[height][width]);
//Reflejar horizontalmente 
void reflect(int height, int width, RGBTRIPLE image[height][width]);
//Filtro de esquinas
void edges(int height, int width, RGBTRIPLE image[height][width]);
//Filtro de blur
void blur(int height, int width, RGBTRIPLE image[height][width]);
//Filtro para pixelear imagenes
void pixelate(int height, int width, RGBTRIPLE image[height][width], int blockSize);
//Filtro de sharpen
void sharpen(int height, int width, RGBTRIPLE image[height][width]);
//Limitar los valores rgb
int cap(int value);
//Aplic el filtro indicado
int apply_filter(unsigned char * rgb_frame, int width, int height, char filter);


#endif