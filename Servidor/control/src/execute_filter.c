#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filtros.h"

int main(int argc, char *argv[])
{
    // Define allowable filters
    char *valid_filters = "bgrsepz";
    char filters[argc]; // Array to store multiple filters
    int filter_count = 0;

    // Parse filter flags
    int opt;
    while ((opt = getopt(argc, argv, valid_filters)) != -1)
    {
        if (opt == '?')
        {
            fprintf(stderr, "Invalid filter.\n");
            return 1;
        }
        filters[filter_count++] = (char)opt;
    }

    // Ensure proper usage
    if (argc != optind + 2)
    {
        fprintf(stderr, "Usage: filter [flags] infile outfile\n");
        return 3;
    }

    // Null-terminate the filters string for safety
    filters[filter_count] = '\0';

    // Remember filenames
    char *infile = argv[optind];
    char *outfile = argv[optind + 1];

    // Open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 4;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 5;
    }

    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Ensure infile is a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 6;
    }

    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Allocate memory for image
    RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (image == NULL)
    {
        fprintf(stderr, "Not enough memory to store image.\n");
        fclose(outptr);
        fclose(inptr);
        return 7;
    }

    // Determine padding for scanlines
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Read the input image into memory
    for (int i = 0; i < height; i++)
    {
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);
        fseek(inptr, padding, SEEK_CUR);
    }

    // Apply each filter sequentially
    for (int i = 0; i < filter_count; i++)
    {
        switch (filters[i])
        {
            case 'b':
                blur2(height, width, image);
                break;
            case 'g':
                grayscale(height, width, image);
                break;
            case 'r':
                reflect(height, width, image);
                break;
            case 's':
                sepia(height, width, image);
                break;
            case 'e':
                edges(height, width, image);
                break;
            case 'p':
                pixelate(height, width, image,4);
                break;
            case 'z':
                sharpen(height, width, image);
                break;
            default:
                fprintf(stderr, "Unknown filter: %c\n", filters[i]);
                return 8;
        }
    }

    // Write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Write the modified image to the output file
    for (int i = 0; i < height; i++)
    {
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Free memory and close files
    free(image);
    fclose(inptr);
    fclose(outptr);

    return 0;
}
