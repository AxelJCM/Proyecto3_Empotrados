#ifndef FILTER_H
#define FILTER_H

#include <stdint.h> // For using uint8_t data type

// Definition of a pixel structure for BMP images
typedef struct
{
    uint8_t rgbtBlue;   // Blue component of the pixel
    uint8_t rgbtGreen;  // Green component of the pixel
    uint8_t rgbtRed;    // Red component of the pixel
} RGBTRIPLE;

// Function to apply grayscale filter to an image
// Converts each pixel to a shade of gray by averaging the red, green, and blue components
void grayScale(int height, int width, RGBTRIPLE image[height][width]);

// Function to apply Gaussian blur filter to an image
// Uses a Gaussian kernel to smooth the image by reducing noise and detail
void gaussianBlur(int height, int width, RGBTRIPLE image[height][width]);

// Function to apply edge detection filter to an image
// Highlights the edges in the image using the Sobel operator for horizontal and vertical gradients
void edgeDetection(int height, int width, RGBTRIPLE image[height][width]);

// Function to process BMP files with a specified filter
// Reads a BMP file, applies the chosen filter, and writes the result to a new BMP file
void processBMP(const char *inputPath, const char *outputPath, void (*filter)(int, int, RGBTRIPLE[height][width]));

#endif // FILTER_H
