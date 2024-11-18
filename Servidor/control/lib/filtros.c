#include "filtros.h"
#include <stdio.h>
#include <math.h>
#include <omp.h>  // Include the OpenMP header for parallelization
// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Read the RGB values from the image
            float blue = image[i][j].rgbtBlue;
            float green = image[i][j].rgbtGreen;
            float red = image[i][j].rgbtRed;

            // Rounding the average to avoid errors i.e. > 255
            float avg = round((blue + green + red) / 3);

            // Assigning each of the RGB value equal to the avg of the RGB value of each pixel
            image[i][j].rgbtBlue = image[i][j].rgbtGreen = image[i][j].rgbtRed = avg;
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Execution time: %f seconds\n", time_taken);
    return;
}

// Convert image to sepia
void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // int thread_id = omp_get_thread_num(); // Get the current thread ID
            // printf("Thread %d processing pixel [%d, %d]\n", thread_id, i, j);
            // Initializing the RGB values from the image
            float blue = image[i][j].rgbtBlue;
            float green = image[i][j].rgbtGreen;
            float red = image[i][j].rgbtRed;

            // Calculating the sepia value from the given formula in the pset
            float sepiaRed = round(.393 * red + .769 * green + .189 * blue);
            float sepiaGreen = round(.349 * red + .686 * green  + .168 * blue);
            float sepiaBlue = round(.272 * red + .534 * green + .131 * blue);

            // Checking if the values are not exceeding 255
            if (sepiaRed > 255)
            {
                sepiaRed = 255;
            }
            if (sepiaBlue > 255)
            {
                sepiaBlue = 255;
            }
            if (sepiaGreen > 255)
            {
                sepiaGreen = 255;
            }

            // Assigning the RGB values of the image to the new calculated sepia values
            image[i][j].rgbtBlue = sepiaBlue;
            image[i][j].rgbtGreen = sepiaGreen;
            image[i][j].rgbtRed = sepiaRed;
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Execution time: %f seconds\n", time_taken);
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        // If done till the 'width' it becomes reflected symmetrically vertically, that is why done till 'width/2'
        for (int j = 0; j < width / 2; j++)
        {
            // Initializing the RGB values from the image
            int blue = image[i][j].rgbtBlue;
            int green = image[i][j].rgbtGreen;
            int red = image[i][j].rgbtRed;

            // Reversing the image horizontally, just like reversing an 1D array
            image[i][j].rgbtBlue = image[i][width - j - 1].rgbtBlue;
            image[i][j].rgbtGreen = image[i][width - j - 1].rgbtGreen;
            image[i][j].rgbtRed = image[i][width - j - 1].rgbtRed;

            // Assigning the newly reverse image with the oriiginal RGB values
            image[i][width - j - 1].rgbtBlue = blue;
            image[i][width - j - 1].rgbtGreen = green;
            image[i][width - j - 1].rgbtRed = red;
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Execution time: %f seconds\n", time_taken);
    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // Creating a temporary image for storing values
    RGBTRIPLE tmp[height][width];
    float blue, green, red, count;

    double start_time = omp_get_wtime();
    //#pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            blue = green = red = count = 0;

            // This loop runs from rows i - 1 to i + 1, i.e. if i = 0, runs from -1 to 1, if i = 2, runs from 1 to 3
            for (int x = i - 1; x <= i + 1; x++)
            {
                // This loop runs from columns j - 1 to j + 1, i.e. if j = 3, j runs from 2 to 4
                for (int y = j - 1; y <= j + 1; y++)
                {
                    // Adding RGB values around 3x3 of the pixel
                    // Check the pixels around
                    if ((x >= 0 && x < height) && (y >= 0 && y < width))
                    {
                        blue += image[x][y].rgbtBlue;
                        green += image[x][y].rgbtGreen;
                        red += image[x][y].rgbtRed;
                        // count records the number of pixels around the particular pixel
                        count++;
                    }
                }
            }

            // Make sure that the values is not divided by zero
            if (count != 0)
            {
                tmp[i][j].rgbtBlue = round(blue / count);
                tmp[i][j].rgbtGreen = round(green / count);
                tmp[i][j].rgbtRed = round(red / count);
            }
            else
            {
                return;
            }
        }
    }

    // Assigning the temporary values to the original image to create the blur
    //#pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = tmp[i][j];
        }
    }


    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    printf("Execution time: %f seconds\n", time_taken);
    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}, Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    RGBTRIPLE temp[height][width];
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            temp[i][j] = image[i][j];
    double start_time = omp_get_wtime();


    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int GxR = 0, GxG = 0, GxB = 0;
            int GyR = 0, GyG = 0, GyB = 0;

            for (int x = i - 1; x <= i + 1; x++)
            {
                for (int y = j - 1; y <= j + 1; y++)
                {
                    if ((x != -1 && x != height) && (y != -1 && y != width))
                    {
                        GxR += temp[x][y].rgbtRed * Gx[x-i+1][y-j+1];
                        GxG += temp[x][y].rgbtGreen * Gx[x-i+1][y-j+1];
                        GxB += temp[x][y].rgbtBlue * Gx[x-i+1][y-j+1];
                        GyR += temp[x][y].rgbtRed * Gy[x-i+1][y-j+1];
                        GyG += temp[x][y].rgbtGreen * Gy[x-i+1][y-j+1];
                        GyB += temp[x][y].rgbtBlue * Gy[x-i+1][y-j+1];
                    }
                }
            }
            image[i][j].rgbtRed = cap(round(sqrt(GxR*GxR + GyR*GyR)));
            image[i][j].rgbtGreen = cap(round(sqrt(GxG*GxG + GyG*GyG)));
            image[i][j].rgbtBlue = cap(round(sqrt(GxB*GxB + GyB*GyB)));
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    printf("Execution time: %f seconds\n", time_taken);
    return;
}
int cap(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return value;
}
void blur2(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE temp[height][width];
    double start_time = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sumBlue = 0, sumGreen = 0, sumRed = 0;
            int count = 0;

            // Recorre los píxeles vecinos en un área de 3x3
            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    // Verifica los límites de la imagen
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width)
                    {
                        sumBlue += image[ni][nj].rgbtBlue;
                        sumGreen += image[ni][nj].rgbtGreen;
                        sumRed += image[ni][nj].rgbtRed;
                        count++;
                    }
                }
            }

            // Asigna los valores promediados al píxel temporal
            temp[i][j].rgbtBlue = round((float)sumBlue / count);
            temp[i][j].rgbtGreen = round((float)sumGreen / count);
            temp[i][j].rgbtRed = round((float)sumRed / count);
        }
    }

    // Copia la imagen procesada de vuelta a la imagen original
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = temp[i][j];
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    printf("Execution time: %f seconds\n", time_taken);
}
// Pixelate image
void pixelate(int height, int width, RGBTRIPLE image[height][width], int blockSize)
{
    double start_time = omp_get_wtime();

    // Iterate over the image in blocks of blockSize x blockSize
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i += blockSize)
    {
        for (int j = 0; j < width; j += blockSize)
        {
            int sumRed = 0, sumGreen = 0, sumBlue = 0;
            int pixelCount = 0;

            // Calculate the average color of the block
            for (int bi = i; bi < i + blockSize && bi < height; bi++)
            {
                for (int bj = j; bj < j + blockSize && bj < width; bj++)
                {
                    sumRed += image[bi][bj].rgbtRed;
                    sumGreen += image[bi][bj].rgbtGreen;
                    sumBlue += image[bi][bj].rgbtBlue;
                    pixelCount++;
                }
            }

            // Calculate average color values
            int avgRed = round((float)sumRed / pixelCount);
            int avgGreen = round((float)sumGreen / pixelCount);
            int avgBlue = round((float)sumBlue / pixelCount);

            // Assign the average color to all pixels in the block
            for (int bi = i; bi < i + blockSize && bi < height; bi++)
            {
                for (int bj = j; bj < j + blockSize && bj < width; bj++)
                {
                    image[bi][bj].rgbtRed = avgRed;
                    image[bi][bj].rgbtGreen = avgGreen;
                    image[bi][bj].rgbtBlue = avgBlue;
                }
            }
        }
    }

    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Execution time: %f seconds\n", time_taken);

    return;
}


// Sharpen image
void sharpen(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();

    RGBTRIPLE temp[height][width];
    
    // Copy original image to temp array
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            temp[i][j] = image[i][j];
        }
    }

    // Sharpen kernel
    int kernel[3][3] = {{ 0, -1,  0 },
                        {-1,  5, -1 },
                        { 0, -1,  0 }};
    
    // Apply kernel to image
    #pragma omp parallel for collapse(2)
    for (int i = 1; i < height - 1; i++)
    {
        for (int j = 1; j < width - 1; j++)
        {
            int newRed = 0, newGreen = 0, newBlue = 0;
            
            for (int ki = -1; ki <= 1; ki++)
            {
                for (int kj = -1; kj <= 1; kj++)
                {
                    newRed += temp[i + ki][j + kj].rgbtRed * kernel[ki + 1][kj + 1];
                    newGreen += temp[i + ki][j + kj].rgbtGreen * kernel[ki + 1][kj + 1];
                    newBlue += temp[i + ki][j + kj].rgbtBlue * kernel[ki + 1][kj + 1];
                }
            }

            // Clamp values to 0-255
            image[i][j].rgbtRed = fmin(fmax(newRed, 0), 255);
            image[i][j].rgbtGreen = fmin(fmax(newGreen, 0), 255);
            image[i][j].rgbtBlue = fmin(fmax(newBlue, 0), 255);
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Execution time: %f seconds\n", time_taken);

    return;
}
