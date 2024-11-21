#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filtros.h"

int main(int argc, char *argv[])
{
    // Filtros válidos que el programa puede aplicar
    char *valid_filters = "bgrsepz";
    // Array para almacenar los filtros seleccionados por el usuario
    char filters[argc]; 
    int filter_count = 0;

    // Procesar los argumentos de línea de comandos para identificar los filtros
    int opt;
    while ((opt = getopt(argc, argv, valid_filters)) != -1)
    {
        if (opt == '?') // Si se encuentra un filtro inválido
        {
            fprintf(stderr, "Invalid filter.\n");
            return 1; // Error por filtro no reconocido
        }
        // Almacenar el filtro válido en el array
        filters[filter_count++] = (char)opt;
    }

    // Verificar que se hayan proporcionado los argumentos requeridos: infile y outfile
    if (argc != optind + 2)
    {
        fprintf(stderr, "Usage: filter [flags] infile outfile\n");
        return 3; // Error por uso incorrecto
    }

    // Terminar el array de filtros con un carácter nulo
    filters[filter_count] = '\0';

    // Almacenar los nombres de los archivos de entrada y salida
    char *infile = argv[optind];
    char *outfile = argv[optind + 1];

    // Abrir el archivo de entrada para lectura
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 4; // Error al abrir el archivo de entrada
    }

    // Abrir el archivo de salida para escritura
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 5; // Error al crear el archivo de salida
    }

    // Leer el encabezado del archivo BMP (BITMAPFILEHEADER)
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Leer el encabezado de información BMP (BITMAPINFOHEADER)
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Obtener las dimensiones de la imagen
    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Reservar memoria para almacenar la imagen completa
    RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (image == NULL)
    {
        fprintf(stderr, "Not enough memory to store image.\n");
        fclose(outptr);
        fclose(inptr);
        return 7; // Error por falta de memoria
    }

    // Calcular el relleno (padding) para las líneas de escaneo
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Leer la imagen del archivo de entrada a la memoria
    for (int i = 0; i < height; i++)
    {
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);
        fseek(inptr, padding, SEEK_CUR); // Saltar el relleno
    }

    // Aplicar cada filtro seleccionado en secuencia
    for (int i = 0; i < filter_count; i++)
    {
        switch (filters[i])
        {
            case 'b':
                blur(height, width, image); // Aplicar desenfoque
                break;
            case 'g':
                grayscale(height, width, image); // Convertir a escala de grises
                break;
            case 'r':
                reflect(height, width, image); // Reflejar horizontalmente
                break;
            case 's':
                sepia(height, width, image); // Aplicar filtro sepia
                break;
            case 'e':
                edges(height, width, image); // Detectar bordes
                break;
            case 'p':
                pixelate(height, width, image, 9); // Pixelar la imagen
                break;
            case 'z':
                sharpen(height, width, image); // Agregar nitidez
                break;
            default:
                fprintf(stderr, "Unknown filter: %c\n", filters[i]);
                return 8; // Error por filtro desconocido
        }
    }

    // Escribir el encabezado del archivo BMP en el archivo de salida
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Escribir el encabezado de información BMP en el archivo de salida
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Escribir la imagen modificada en el archivo de salida
    for (int i = 0; i < height; i++)
    {
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);
        // Escribir el relleno
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Liberar memoria y cerrar los archivos abiertos
    free(image);
    fclose(inptr);
    fclose(outptr);

    return 0; // Éxito
}
