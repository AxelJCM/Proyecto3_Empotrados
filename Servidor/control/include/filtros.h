#include "bmp.h"

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width]);

// Convert image to sepia
void sepia(int height, int width, RGBTRIPLE image[height][width]);

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width]);

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width]);

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width]);

void blur2(int height, int width, RGBTRIPLE image[height][width]);
void pixelate(int height, int width, RGBTRIPLE image[height][width], int blockSize);
void sharpen(int height, int width, RGBTRIPLE image[height][width]);


int cap(int value);