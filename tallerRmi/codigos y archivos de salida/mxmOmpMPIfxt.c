/************************************************************************************************
                                = Pontificia Universidad Javeriana =

    Autores:
        - Juan Carlos Santamaria Orjuela
        - Juan Felipe Gutiérrez Adarme
        - Juan Daza
        - Juan Rincón

    Basado en:
        - John Corredor (Versión prototipo inicial)
   
            - Integración con OpenMP

    DESCRIPCIÓN GENERAL:
        Este programa realiza la multiplicación de matrices cuadradas (NxN)
        usando paralelismo híbrido:

        - MPI: distribuye el trabajo entre procesos (nodos)
        - OpenMP: paraleliza el cálculo dentro de cada proceso

        A diferencia de la versión FxC, aquí se usa:
        Transposición de la matriz B (mejor uso de caché)

        Flujo:
        1. MASTER divide matrixA en bloques
        2. Envía bloques a WORKERS
        3. Todos reciben matrixB
        4. WORKERS calculan con OpenMP (usando transpuesta)
        5. Envían resultados al MASTER
        6. MASTER reconstruye matrixC

************************************************************************************************/

#include <mpi.h>        // Librería MPI
#include <stdlib.h>     // malloc, calloc, free
#include "moduloMPI.h" // funciones auxiliares

#define MASTER 0        // ID del proceso maestro
#define FROM_MASTER 1   // Tag (no crítico aquí)
#define FROM_WORKER 2   // Tag (no crítico aquí)

int main(int argc, char *argv[]) {

/****************************************************************************************************
=           0.- Validación de argumentos de entrada                                                  =
****************************************************************************************************/

    argumentos(argc); // Verifica cantidad de argumentos

    int N  = (int) atoi(argv[1]);   // Dimensión de matrices (NxN)
    int nH = (int) atoi(argv[2]);   // Número de hilos OpenMP

    // Punteros de matrices (usados por MASTER)
    double *matrixA = NULL, *matrixB = NULL, *matrixC = NULL;

    int cantProcesos;   // Número total de procesos
    int idProceso;      // ID del proceso actual
    int tW;             // Filas por worker

/****************************************************************************************************
=           1.- Inicialización del entorno MPI                                                       =
****************************************************************************************************/

    MPI_Init(&argc, &argv); // Inicializa MPI

    MPI_Comm_size(MPI_COMM_WORLD, &cantProcesos); // Total procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &idProceso);    // ID proceso

    int cantidadW = cantProcesos - 1; // Workers = total - MASTER

/****************************************************************************************************
=           2.- MASTER: inicialización de datos                                                      =
****************************************************************************************************/

    if (idProceso == MASTER) {

        // Verifica divisibilidad de la matriz
        verificarDiv(cantidadW, N);

        // Reserva memoria para matrices completas
        matrixA = (double *)calloc(N*N, sizeof(double));
        matrixB = (double *)calloc(N*N, sizeof(double));
        matrixC = (double *)calloc(N*N, sizeof(double));

        // Inicializa matrices A y B
        iniMatrix(matrixA, matrixB, N);

        // Calcula tamaño de bloque por worker
        tW = N / cantidadW;

        // Debug (solo matrices pequeñas)
        impMatrix(matrixA, N);
        impMatrix(matrixB, N);
        mensajeVerifica(N, cantidadW);
    }

/****************************************************************************************************
=           3.- Broadcast de parámetros globales                                                     =
****************************************************************************************************/

    MPI_Bcast(&N,  1, MPI_INT, 0, MPI_COMM_WORLD);  // Dimensión
    MPI_Bcast(&tW, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Filas por worker

/****************************************************************************************************
=           4.- MASTER: distribución del trabajo                                                     =
****************************************************************************************************/

    if (idProceso == MASTER) {

        iniTime(); // Inicio medición de tiempo

        // Enviar matrixB completa a todos
        MPI_Bcast(matrixB, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Enviar bloques de matrixA
        for (int w = 1; w < cantProcesos; w++) {
            int iniSlice = (w-1)*tW; // inicio del bloque

            MPI_Send(matrixA + iniSlice*N, tW*N, MPI_DOUBLE,
                     w, 0, MPI_COMM_WORLD);
        }

        // Recibir resultados
        for (int w = 1; w < cantProcesos; w++) {
            int iniSlice = (w-1)*tW;

            MPI_Recv(matrixC + iniSlice*N, tW*N, MPI_DOUBLE,
                     w, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        endTime(); // Fin medición

        // Imprimir resultado si es pequeño
        impMatrix(matrixC, N);
    }

/****************************************************************************************************
=           5.- WORKERS: recepción y cálculo                                                         =
****************************************************************************************************/

    if (idProceso > MASTER){

        // Reservar memoria local
        double *matA = (double *)malloc(tW*N*sizeof(double));
        double *matB = (double *)malloc(N*N*sizeof(double));
        double *matC = (double *)malloc(tW*N*sizeof(double));

        // Recibir matrixB
        MPI_Bcast(matB, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Recibir bloque de A
        MPI_Recv(matA, tW*N, MPI_DOUBLE,
                 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

/****************************************************************************************************
=           6.- Cálculo paralelo con OpenMP (con transposición)                                     =
****************************************************************************************************/

        // Multiplicación usando versión optimizada (transpuesta)
        mxmOmpFxT(matA, matB, matC, tW, N, nH);

        // Enviar resultado al MASTER
        MPI_Send(matC, tW*N, MPI_DOUBLE,
                 0, 1, MPI_COMM_WORLD);

        // Liberar memoria
        free(matA);
        free(matC);
        free(matB);
    }

/****************************************************************************************************
=           7.- MASTER: liberación de memoria                                                        =
****************************************************************************************************/

    if (idProceso == MASTER) {
        free(matrixA);
        free(matrixB);
        free(matrixC);
    }

    MPI_Finalize(); // Finaliza MPI

    return 0;
}
