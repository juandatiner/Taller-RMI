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
        Este programa implementa la multiplicación de matrices cuadradas (NxN)
        utilizando paralelismo híbrido:

        - MPI: distribuye el trabajo entre múltiples procesos (nodos)
        - OpenMP: paraleliza el cálculo dentro de cada proceso

        Estrategia:
        1. El proceso MASTER divide la matriz A en bloques (filas)
        2. Envía cada bloque a los WORKERS
        3. Todos reciben la matriz B completa
        4. Cada WORKER calcula su parte con OpenMP
        5. Se envían los resultados al MASTER
        6. El MASTER reconstruye la matriz resultado

    Ejecución:
        $ mpirun -hostfile fileHOSTSMPI -np 3 ./ejecutable N numHilos

************************************************************************************************/

#include <mpi.h>        // Librería MPI (paralelismo distribuido)
#include <stdlib.h>     // malloc, calloc, free
#include "moduloMPI.h" // Funciones auxiliares

#define MASTER 0        // ID del proceso MASTER
#define FROM_MASTER 1   // Tag (no usado explícitamente aquí)
#define FROM_WORKER 2   // Tag (no usado explícitamente aquí)

int main(int argc, char *argv[]) {

/****************************************************************************************************
=           0.- Validación de argumentos de entrada                                                  =
****************************************************************************************************/
    argumentos(argc);

    int N  = (int) atoi(argv[1]);   // Dimensión de matrices (NxN)
    int nH = (int) atoi(argv[2]);   // Número de hilos OpenMP

    // Punteros para matrices (solo usados por MASTER inicialmente)
    double *matrixA = NULL, *matrixB = NULL, *matrixC = NULL;

    int cantProcesos;   // Número total de procesos MPI
    int idProceso;      // ID del proceso actual
    int tW;             // Tamaño de bloque (filas por worker)

/****************************************************************************************************
=           1.- Inicialización del entorno MPI                                                       =
****************************************************************************************************/
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &cantProcesos); // Total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &idProceso);    // ID del proceso

    int cantidadW = cantProcesos - 1; // Workers = total - MASTER

    /* ================= CASO SECUENCIAL ================= */
    if (cantProcesos == 1) {

        if (idProceso == MASTER) {

            // Reserva memoria completa
            matrixA = (double *)calloc(N*N, sizeof(double));
            matrixB = (double *)calloc(N*N, sizeof(double));
            matrixC = (double *)calloc(N*N, sizeof(double));

            // Inicializa matrices
            iniMatrix(matrixA, matrixB, N);

            // Medición de tiempo
            iniTime();

            // Ejecuta multiplicación completa sin MPI
            mxmOmpFxC(matrixA, matrixB, matrixC, N, N, nH);

            endTime();

            // Mostrar resultado si es pequeño
            impMatrix(matrixC, N);

            // Liberar memoria
            free(matrixA);
            free(matrixB);
            free(matrixC);
        }

        MPI_Finalize();
        return 0;
    }

/****************************************************************************************************
=           2.- MASTER: inicialización de datos                                                      =
****************************************************************************************************/
    if (idProceso == MASTER) {

        if (cantProcesos > 1) {
            verificarDiv(cantidadW, N);
        }

        // Reserva memoria para matrices completas
        matrixA = (double *)calloc(N*N, sizeof(double));
        matrixB = (double *)calloc(N*N, sizeof(double));
        matrixC = (double *)calloc(N*N, sizeof(double));

        // Inicializa matrices A y B
        iniMatrix(matrixA, matrixB, N);

        // Número de filas por worker
        tW = N / cantidadW;

        // Debug (solo matrices pequeñas)
        impMatrix(matrixA, N);
        impMatrix(matrixB, N);
        mensajeVerifica(N, cantidadW);
    }

/****************************************************************************************************
=           3.- Broadcast de parámetros globales                                                     =
****************************************************************************************************/
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Dimensión
    MPI_Bcast(&tW, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Filas por worker

/****************************************************************************************************
=           4.- MASTER: distribución del trabajo                                                     =
****************************************************************************************************/
    if (idProceso == MASTER) {

        // Inicio de medición de tiempo
        iniTime();

        // Enviar matriz B completa a todos
        MPI_Bcast(matrixB, N*N, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

        // Enviar bloques de matrixA a cada worker
        for (int w = 1; w < cantProcesos; w++) {
            int iniSlice = (w - 1) * tW; // inicio del bloque

            MPI_Send(matrixA + iniSlice*N, tW*N, MPI_DOUBLE,
                     w, 0, MPI_COMM_WORLD);
        }

        // Recibir resultados parciales
        for (int w = 1; w < cantProcesos; w++) {
            int iniSlice = (w - 1) * tW;

            MPI_Recv(matrixC + iniSlice*N, tW*N, MPI_DOUBLE,
                     w, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Fin de medición de tiempo
        endTime();

        // Mostrar resultado si es pequeño
        impMatrix(matrixC, N);
    }

/****************************************************************************************************
=           5.- WORKERS: recepción y cálculo                                                         =
****************************************************************************************************/
    if (idProceso > MASTER) {

        // Reservar memoria local
        double *matA = (double *)malloc(tW*N*sizeof(double));
        double *matB = (double *)malloc(N*N*sizeof(double));
        double *matC = (double *)malloc(tW*N*sizeof(double));

        // Recibir matriz B
        MPI_Bcast(matB, N*N, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

        // Recibir bloque de A
        MPI_Recv(matA, tW*N, MPI_DOUBLE,
                 MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

/****************************************************************************************************
=           6.- Cálculo paralelo con OpenMP                                                          =
****************************************************************************************************/

        // Multiplicación de matrices (versión filas x columnas)
        mxmOmpFxC(matA, matB, matC, tW, N, nH);

        // Enviar resultado al MASTER
        MPI_Send(matC, tW*N, MPI_DOUBLE,
                 MASTER, 1, MPI_COMM_WORLD);

        // Liberar memoria
        free(matA);
        free(matB);
        free(matC);
    }

/****************************************************************************************************
=           7.- Liberación de memoria (MASTER)                                                       =
****************************************************************************************************/
    if (idProceso == MASTER) {
        free(matrixA);
        free(matrixB);
        free(matrixC);
    }

    // Finaliza entorno MPI
    MPI_Finalize();

    return 0;
}
