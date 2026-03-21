/************************************************************************************************
                                = Pontificia Universidad Javeriana =

   Autores:
        - Juan Carlos Santamaria Orjuela
        - Juan Felipe Gutiérrez Adarme
        - Juan Daza
        - Juan Rincón

    Basado en:
        - John Corredor (Versión prototipo inicial)
    

    Descripción GENERAL DEL ARCHIVO:
        Este archivo contiene funciones auxiliares para la multiplicación de matrices
        utilizando paralelismo híbrido:
        - MPI (memoria distribuida entre nodos)
        - OpenMP (paralelismo dentro de cada nodo)

        Incluye:
        - Inicialización de matrices
        - Multiplicación de matrices (dos versiones)
        - Transposición de matrices
        - Medición de tiempo
        - Validaciones y utilidades de impresión

************************************************************************************************/

#include <mpi.h>        // Librería principal de MPI
#include <omp.h>        // Librería para paralelismo con OpenMP
#include <stdio.h>      // Entrada y salida estándar (printf)
#include <stdlib.h>     // Funciones de memoria dinámica (calloc, exit)
#include <time.h>       // Manejo de tiempo (rand, srand)
#include <sys/time.h>   // Medición de tiempo de alta precisión
#include "moduloMPI.h" // Archivo de cabecera propio

// Variables globales para medir tiempo de ejecución
struct timeval inicio, fin;


/************************************************************************************************
    Función: impMatrix
    Descripción:
        Imprime una matriz solo si su tamaño es pequeño (n < 13)
************************************************************************************************/
void impMatrix(double *mat, int n){
    if(n<13){
        printf("\n====================================================================");
        // Recorre toda la matriz como arreglo lineal
        for (int i = 0; i < n*n; i++, mat++) {
            if(i%n == 0 ) printf("\n"); // Salto de línea cada fila
            printf("%0.3f ", *mat);
        }
        printf("\n====================================================================\n");
    }
}


/************************************************************************************************
    Función: matrixTRP
    Descripción:
        Calcula la transpuesta de la matriz mB y la guarda en mT
************************************************************************************************/
void matrixTRP(int N, double *mB, double *mT){
    for(int i=0; i<N; i++)
        for(int j=0; j<N; j++)
            mT[i*N+j] = mB[j*N+i]; // Intercambio de filas por columnas

    // Imprime la matriz transpuesta (solo si es pequeña)
    impMatrix(mT, N);
}


/************************************************************************************************
    Función: mxmOmpFxT
    Descripción:
        Multiplicación de matrices usando:
        - Transposición previa de la matriz B (mejor acceso a memoria)
        - Paralelismo con OpenMP

    Parámetros:
        mA -> matriz A
        mB -> matriz B
        mC -> matriz resultado
        tw -> número de filas asignadas al worker
        D  -> dimensión de la matriz (NxN)
        nH -> número de hilos OpenMP
************************************************************************************************/
void mxmOmpFxT(double *mA, double *mB, double *mC, int tw, int D, int nH){
    // Reserva memoria para la matriz transpuesta
    double *mT  = (double *)calloc(D*D, sizeof(double));

    // Transpone la matriz B
    matrixTRP(D, mB, mT);

    // Define el número de hilos
    omp_set_num_threads(nH);

    #pragma omp parallel
    {
        #pragma omp for
        // Recorre las filas asignadas
        for(int i=0; i<tw; i++)
            for(int j=0; j<D; j++){
                double *pA, *pB, Suma = 0.0;

                // Punteros a la fila de A y fila de la transpuesta
                pA = mA+i*D;
                pB = mT+j*D;

                // Producto punto
                for(int k=0; k<D; k++, pA++, pB++)
                    Suma += *pA * *pB;

                // Guarda resultado
                mC[i*D+j] = Suma;
            }
    }

    // Libera memoria
    free(mT);
}


/************************************************************************************************
    Función: mxmOmpFxC
    Descripción:
        Multiplicación de matrices sin transponer B
        (acceso por columnas, menos eficiente en caché)
************************************************************************************************/
void mxmOmpFxC(double *mA, double *mB, double *mC, int tw, int D, int nH){

    omp_set_num_threads(nH);

    #pragma omp parallel
    {
        #pragma omp for
        for(int i=0; i<tw; i++)
            for(int j=0; j<D; j++){
                double *pA, *pB, Suma = 0.0;

                pA = mA+i*D; // fila de A
                pB = mB+j;   // columna de B

                // Producto punto (acceso columna por columna)
                for(int k=0; k<D; k++, pA++, pB+=D)
                    Suma += *pA * *pB;

                mC[i*D+j] = Suma;
            }
    }
}


/************************************************************************************************
    Función: mensajeVerifica
    Descripción:
        Imprime información de verificación del sistema (debug)
************************************************************************************************/
void mensajeVerifica(int N, int cantidadW){
    if(N<13){
        printf("\n");
        printf("********************************************************************\n");
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("++++++ \t\tDimensión de Matrix NxN \t  = %dx%d \t++++\n", N, N);
        printf("++++++ \t\tCantidad de Workers (np - MASTER) = %d \t\t++++\n", cantidadW);
        printf("++++++ \t\tTajada de matriz A para Workers   = %dx%d \t++++\n", N/cantidadW,N);
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("********************************************************************\n");
        printf("\n");
    }
}


/************************************************************************************************
    Función: iniMatrix
    Descripción:
        Inicializa matrices A y B con valores (actualmente determinísticos)
************************************************************************************************/
void iniMatrix(double *mA, double *mB, int D){
    srand(time(NULL)); // Inicializa semilla aleatoria

    for(int i=0; i<D*D; i++, mA++, mB++){
        // Valores secuenciales (útil para pruebas)
        *mA = 0.08*i;
        *mB = 0.02*i;
    }
}


/************************************************************************************************
    Función: iniTime
    Descripción:
        Marca el inicio de la medición de tiempo
************************************************************************************************/
void iniTime(){
    gettimeofday(&inicio, (void *)0);
}


/************************************************************************************************
    Función: endTime
    Descripción:
        Calcula e imprime el tiempo transcurrido en microsegundos
************************************************************************************************/
void endTime(){
    gettimeofday(&fin, (void *)0);

    // Calcula diferencia de tiempo
    fin.tv_usec -= inicio.tv_usec;
    fin.tv_sec  -= inicio.tv_sec;

    double tiempo = (double) (fin.tv_sec*1000000 + fin.tv_usec);

    printf("%9.0f \n", tiempo);
}


/************************************************************************************************
    Función: argumentos
    Descripción:
        Verifica que el número de argumentos sea correcto
************************************************************************************************/
void argumentos(int cantidad){
    if (cantidad != 3){
        printf("Ingreso de Argumentos: \n\n");
        printf("\t\t$mpirun -hostfile file -np p ./ejecutable DimMatriz NumHilos \n\n");
        printf("\nfile: Archivo de Master y Workers \n");
        printf("\np: procesos Master+Workers\n");
        exit(0);
    }
}


/************************************************************************************************
    Función: verificarDiv
    Descripción:
        Verifica que la dimensión de la matriz sea divisible entre los workers
************************************************************************************************/
void verificarDiv(int qworkers, int Dim){

    // Validación de divisibilidad
    if ((qworkers < 1) || (Dim%qworkers != 0)) {
        printf("Error: NxN (%d) debe ser divisible por cantidad de workers (%d)\n", Dim, qworkers);
        printf("Error: Número de procesos (%d) > 1 \n", qworkers);

        // Termina todos los procesos MPI
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(0);
    }
}
