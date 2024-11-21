#include "filtros.h"
#include <stdio.h>
#include <math.h>
#include <omp.h>  

// Convierte la imagen a escala de grises
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Lee los valores RGB del píxel
            float blue = image[i][j].rgbtBlue;
            float green = image[i][j].rgbtGreen;
            float red = image[i][j].rgbtRed;

            // Calcula el promedio de los valores RGB y lo redondea
            float avg = round((blue + green + red) / 3);

            // Asigna el promedio como nuevo valor para los componentes RGB del píxel
            image[i][j].rgbtBlue = image[i][j].rgbtGreen = image[i][j].rgbtRed = avg;
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Tiempo de ejecución escala de grises: %f segundos\n", time_taken);
    return;
}

// Convierte la imagen a un tono sepia
void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //Inicializa los valores RGB del píxel
            float blue = image[i][j].rgbtBlue;
            float green = image[i][j].rgbtGreen;
            float red = image[i][j].rgbtRed;

            // Calcula los valores sepia
            float sepiaRed = round(.393 * red + .769 * green + .189 * blue);
            float sepiaGreen = round(.349 * red + .686 * green  + .168 * blue);
            float sepiaBlue = round(.272 * red + .534 * green + .131 * blue);

            // Asegura que los valores no excedan 255
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
            // Asigna los valores sepia al píxel
            image[i][j].rgbtBlue = sepiaBlue;
            image[i][j].rgbtGreen = sepiaGreen;
            image[i][j].rgbtRed = sepiaRed;
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Print the execution time
    printf("Tiempo de ejecución sepia: %f segundos\n", time_taken);
    return;
}

// Refleja la imagen horizontalmente
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        //iterar hasta la mitad del ancho para reflejar horizontalmente
        for (int j = 0; j < width / 2; j++)
        {
            // Guarda los valores del píxel actual
            int blue = image[i][j].rgbtBlue;
            int green = image[i][j].rgbtGreen;
            int red = image[i][j].rgbtRed;
            // Intercambia los píxeles de los extremos
            image[i][j].rgbtBlue = image[i][width - j - 1].rgbtBlue;
            image[i][j].rgbtGreen = image[i][width - j - 1].rgbtGreen;
            image[i][j].rgbtRed = image[i][width - j - 1].rgbtRed;
            // Asigna los valores originales al píxel reflejado
            image[i][width - j - 1].rgbtBlue = blue;
            image[i][width - j - 1].rgbtGreen = green;
            image[i][width - j - 1].rgbtRed = red;
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    printf("Tiempo de ejecución reflejar: %f segundos\n", time_taken);
    return;
}

// Detecta bordes en la imagen usando Sobel
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    // Definición del Kernel de Sobel 
    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}, Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    // Copia la imagen original en un buffer temporal
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
            // Variables para almacenar los resultados de Gx y Gy
            int GxR = 0, GxG = 0, GxB = 0;
            int GyR = 0, GyG = 0, GyB = 0;
            // Itera sobre los píxeles vecinos
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
            // Calcula la magnitud del gradiente y limita los valores a 0-255
            image[i][j].rgbtRed = cap(round(sqrt(GxR*GxR + GyR*GyR)));
            image[i][j].rgbtGreen = cap(round(sqrt(GxG*GxG + GyG*GyG)));
            image[i][j].rgbtBlue = cap(round(sqrt(GxB*GxB + GyB*GyB)));
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    printf("Tiempo de ejecución Borde Sobel: %f segundos\n", time_taken);
    return;
}
// Función auxiliar para limitar los valores entre 0 y 255
int cap(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return value;
}

//Es un blur de tipo mean blur es decir su kernel es de 3x3 con valores de 1
//Tambien cono conocido como box blur 
void blur(int height, int width, RGBTRIPLE image[height][width])
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
    printf("Tiempo de ejecución Blur: %f segundos\n", time_taken);
}

// Pixelate image
void pixelate(int height, int width, RGBTRIPLE image[height][width], int blockSize)
{
    double start_time = omp_get_wtime();

    // Itera sobre la imagen en bloques de tamaño blockSize x blockSize
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i += blockSize)
    {
        for (int j = 0; j < width; j += blockSize)
        {
            // Variables para acumular los valores de color y contar los píxeles en el bloque
            int sumRed = 0, sumGreen = 0, sumBlue = 0;
            int pixelCount = 0;

             // Calcula el color promedio del bloque           
            for (int bi = i; bi < i + blockSize && bi < height; bi++)
            {
                for (int bj = j; bj < j + blockSize && bj < width; bj++)
                {
                    // Acumula los valores de los colores rojo, verde y azul
                    sumRed += image[bi][bj].rgbtRed;
                    sumGreen += image[bi][bj].rgbtGreen;
                    sumBlue += image[bi][bj].rgbtBlue;
                    pixelCount++;
                }
            }

            // Calcula los valores promedio de los colores del bloque
            int avgRed = round((float)sumRed / pixelCount);
            int avgGreen = round((float)sumGreen / pixelCount);
            int avgBlue = round((float)sumBlue / pixelCount);

            // Asigna el color promedio a todos los pixles del bloque
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

    printf("Tiempo de ejecución Pixalar: %f segundos\n", time_taken);
    return;
}


// Sharpen image
void sharpen(int height, int width, RGBTRIPLE image[height][width])
{
    double start_time = omp_get_wtime();

    RGBTRIPLE temp[height][width];
    
    // Copiar la img original al temp array
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
    
    // Aplicar el kernel a la imagen
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
        

            image[i][j].rgbtRed = cap(newRed);
            image[i][j].rgbtGreen = cap(newGreen);
            image[i][j].rgbtBlue = cap(newBlue);
        }
    }
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    printf("Tiempo de ejecución Sharpen: %f segundos\n", time_taken);

    return;
}
