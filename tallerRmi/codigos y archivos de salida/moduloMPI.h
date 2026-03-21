/************************************************************************************************
                                = Pontificia Universidad Javeriana =

    Autores:
        - Juan Carlos Santamaria Orjuela
        - Juan Felipe Gutiérrez Adarme
        - Juan Daza
        - Juan Rincón

    Basado en:
        - John Corredor (Versión prototipo inicial)
   

    DESCRIPCIÓN GENERAL DEL ARCHIVO:
        Este archivo de cabecera (.h) define las funciones utilizadas en el módulo
        de multiplicación de matrices con paralelismo híbrido:

        - MPI: para distribución del trabajo entre nodos (memoria distribuida)
        - OpenMP: para paralelismo dentro de cada nodo (memoria compartida)

        Aquí SOLO se declaran las funciones (prototipos), no se implementan.
        Las implementaciones están en el archivo moduloMPI.c

************************************************************************************************/

#ifndef __MODULOMPI_H__   // Evita inclusión múltiple del archivo
#define __MODULOMPI_H__


/************************************************************************************************
    Función: matrixTRP
    Descripción:
        Calcula la transpuesta de una matriz.

    Parámetros:
        N  -> dimensión de la matriz (NxN)
        mB -> matriz original
        mT -> matriz donde se guarda la transpuesta
************************************************************************************************/
void matrixTRP(int N, double *mB, double *mT);


/************************************************************************************************
    Función: mxmOmpFxT
    Descripción:
        Multiplicación de matrices usando transposición de la matriz B
        para mejorar el acceso a memoria (cache-friendly) + OpenMP.

    Parámetros:
        mA -> matriz A
        mB -> matriz B
        mC -> matriz resultado
        tw -> filas asignadas al worker
        D  -> dimensión de la matriz
        nH -> número de hilos OpenMP
************************************************************************************************/
void mxmOmpFxT(double *mA, double *mB, double *mC, int tw, int D, int nH);


/************************************************************************************************
    Función: mxmOmpFxC
    Descripción:
        Multiplicación de matrices sin transponer B (forma clásica
        fila por columna). Menos eficiente en caché.

    Parámetros:
        mA -> matriz A
        mB -> matriz B
        mC -> matriz resultado
        tw -> filas asignadas al worker
        D  -> dimensión de la matriz
        nH -> número de hilos OpenMP
************************************************************************************************/
void mxmOmpFxC(double *mA, double *mB, double *mC, int tw, int D, int nH);


/************************************************************************************************
    Función: impMatrix
    Descripción:
        Imprime una matriz en consola (solo para tamaños pequeños).

    Parámetros:
        matA -> matriz a imprimir
        n    -> dimensión
************************************************************************************************/
void impMatrix(double *matA, int n);


/************************************************************************************************
    Función: iniMatrix
    Descripción:
        Inicializa matrices con valores (para pruebas o ejecución).

    Parámetros:
        mA -> matriz A
        mB -> matriz B
        D  -> dimensión
************************************************************************************************/
void iniMatrix(double *mA, double *mB, int D);


/************************************************************************************************
    Función: iniTime
    Descripción:
        Inicia medición de tiempo.
************************************************************************************************/
void iniTime();


/************************************************************************************************
    Función: endTime
    Descripción:
        Finaliza medición de tiempo e imprime resultado.
************************************************************************************************/
void endTime();


/************************************************************************************************
    Función: argumentos
    Descripción:
        Verifica que los argumentos de entrada sean correctos.

    Parámetros:
        cantidad -> número de argumentos recibidos
************************************************************************************************/
void argumentos(int cantidad);


/************************************************************************************************
    Función: verificarDiv
    Descripción:
        Verifica que la dimensión de la matriz sea divisible entre
        el número de workers.

    Parámetros:
        qworkers -> cantidad de workers
        Dim      -> dimensión de la matriz
************************************************************************************************/
void verificarDiv(int qworkers, int Dim);


/************************************************************************************************
    Función: mensajeVerifica
    Descripción:
        Imprime información de verificación (debug del sistema).

    Parámetros:
        N         -> dimensión de la matriz
        cantidadW -> número de workers
************************************************************************************************/
void mensajeVerifica(int N, int cantidadW);


#endif   // Fin del include guard
