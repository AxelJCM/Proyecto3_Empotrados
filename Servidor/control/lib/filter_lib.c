#include "filter_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

//Defining structures to handle the BMP format
#pragma pack(push, 1)
typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct
{
    uint8_t rgbtBlue;
    uint8_t rgbtGreen;
    uint8_t rgbtRed;
} RGBTRIPLE;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s filtro archivo_entrada.bmp archivo_salida.bmp\n", argv[0]);
        fprintf(stderr, "Filtros disponibles: grayscale, gaussian, edges\n");
        return 1;
    }

    //Determine the filter
    void (*filter)(int, int, RGBTRIPLE[height][width]) = NULL;

    if (strcmp(argv[1], "grayscale") == 0)
    {
        filter = grayScale;
    }
    else if (strcmp(argv[1], "gaussian") == 0)
    {
        filter = gaussianBlur;
    }
    else if (strcmp(argv[1], "edges") == 0)
    {
        filter = edgeDetection;
    }
    else
    {
        fprintf(stderr, "Filtro desconocido: %s\n", argv[1]);
        return 2;
    }

    //Process the image
    processBMP(argv[2], argv[3], filter);

    return 0;
}

// Implementation of the function to process BMP
void processBMP(const char *inputPath, const char *outputPath, void (*filter)(int, int, RGBTRIPLE[height][width]))
{
    //Open input file
    FILE *inputFile = fopen(inputPath, "rb");
    if (!inputFile)
    {
        fprintf(stderr, "No se pudo abrir el archivo de entrada.\n");
        exit(3);
    }

    // Open output file
    FILE *outputFile = fopen(outputPath, "wb");
    if (!outputFile)
    {
        fclose(inputFile);
        fprintf(stderr, "No se pudo crear el archivo de salida.\n");
        exit(4);
    }

    //Read BMP headers
    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, inputFile);
    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, inputFile);

    // Validate BMP format
    if (fileHeader.bfType != 0x4D42 || infoHeader.biBitCount != 24 || infoHeader.biCompression != 0)
    {
        fclose(inputFile);
        fclose(outputFile);
        fprintf(stderr, "El archivo no es un BMP de 24 bits no comprimido.\n");
        exit(5);
    }

    // Write headers to output file
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, outputFile);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, outputFile);

    // Determine pixel padding
    int padding = (4 - (infoHeader.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Reserve memory for pixels
    RGBTRIPLE(*image)[infoHeader.biWidth] = calloc(infoHeader.biHeight, infoHeader.biWidth * sizeof(RGBTRIPLE));
    if (!image)
    {
        fclose(inputFile);
        fclose(outputFile);
        fprintf(stderr, "No se pudo asignar memoria.\n");
        exit(6);
    }

    // Read the pixels of the image
    for (int i = 0; i < infoHeader.biHeight; i++)
    {
        fread(image[i], sizeof(RGBTRIPLE), infoHeader.biWidth, inputFile);
        fseek(inputFile, padding, SEEK_CUR);
    }

    // Apply the filter
    filter(infoHeader.biHeight, infoHeader.biWidth, image);

    // Write pixels to output file
    for (int i = 0; i < infoHeader.biHeight; i++)
    {
        fwrite(image[i], sizeof(RGBTRIPLE), infoHeader.biWidth, outputFile);
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outputFile);
        }
    }

    // Free up memory and close files
    free(image);
    fclose(inputFile);
    fclose(outputFile);

    printf("Imagen procesada y guardada en '%s'.\n", outputPath);
}

// Implementación del filtro de escala de grises
void grayScale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int average = round((image[i][j].rgbtRed + image[i][j].rgbtGreen + image[i][j].rgbtBlue) / 3.0);
            image[i][j].rgbtRed = average;
            image[i][j].rgbtGreen = average;
            image[i][j].rgbtBlue = average;
        }
    }
}

// Gaussian blur filter implementation
void gaussianBlur(int height, int width, RGBTRIPLE image[height][width])
{
    // Kernel de suavizado gaussiano
    int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };

    int kernelSum = 16;

    RGBTRIPLE temp[height][width];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sumRed = 0, sumGreen = 0, sumBlue = 0;

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni >= 0 && ni < height && nj >= 0 && nj < width)
                    {
                        sumRed += image[ni][nj].rgbtRed * kernel[di + 1][dj + 1];
                        sumGreen += image[ni][nj].rgbtGreen * kernel[di + 1][dj + 1];
                        sumBlue += image[ni][nj].rgbtBlue * kernel[di + 1][dj + 1];
                    }
                }
            }

            temp[i][j].rgbtRed = sumRed / kernelSum;
            temp[i][j].rgbtGreen = sumGreen / kernelSum;
            temp[i][j].rgbtBlue = sumBlue / kernelSum;
        }
    }

    // Copiar los valores suavizados a la imagen original
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = temp[i][j];
        }
    }
}

// Edge detection filter implementation
void edgeDetection(int height, int width, RGBTRIPLE image[height][width])
{
    // Kernels de Sobel
    int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    RGBTRIPLE temp[height][width];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sumRedX = 0, sumGreenX = 0, sumBlueX = 0;
            int sumRedY = 0, sumGreenY = 0, sumBlueY = 0;

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni >= 0 && ni < height && nj >= 0 && nj < width)
                    {
                        int weightX = gx[di + 1][dj + 1];
                        int weightY = gy[di + 1][dj + 1];

                        sumRedX += image[ni][nj].rgbtRed * weightX;
                        sumGreenX += image[ni][nj].rgbtGreen * weightX;
                        sumBlueX += image[ni][nj].rgbtBlue * weightX;

                        sumRedY += image[ni][nj].rgbtRed * weightY;
                        sumGreenY += image[ni][nj].rgbtGreen * weightY;
                        sumBlueY += image[ni][nj].rgbtBlue * weightY;
                    }
                }
            }

            int red = sqrt(sumRedX * sumRedX + sumRedY * sumRedY);
            int green = sqrt(sumGreenX * sumGreenX + sumGreenY * sumGreenY);
            int blue = sqrt(sumBlueX * sumBlueX + sumBlueY * sumBlueY);

            temp[i][j].rgbtRed = red > 255 ? 255 : red;
            temp[i][j].rgbtGreen = green > 255 ? 255 : green;
            temp[i][j].rgbtBlue = blue > 255 ? 255 : blue;
        }
    }

    // Copy the detected edge values ​​to the original image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = temp[i][j];
        }
    }
}

