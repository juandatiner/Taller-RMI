# Makefile para compilar programas MPI con soporte de OpenMP
# Este archivo automatiza la compilación de dos programas que realizan
# multiplicación de matrices usando MPI y OpenMP.

# Compilador MPI a utilizar
MPICC = mpicc

# Flags de compilación:
# -lm: enlaza la librería matemática
# -fopenmp: habilita el soporte para OpenMP (paralelismo en memoria compartida)
FLAGS = -lm -fopenmp

# Archivo módulo adicional que se incluye en la compilación
MODULO = moduloMPI.c

# Lista de programas a compilar
PROGRAMAS = mxmOmpMPIfxc mxmOmpMPIfxt 

# Regla principal: compila todos los programas definidos en PROGRAMAS
all: ${PROGRAMAS}

# Regla para compilar el programa mxmOmpMPIfxc
# $@ representa el nombre del objetivo (mxmOmpMPIfxc)
# $@.c indica el archivo fuente correspondiente (mxmOmpMPIfxc.c)
mxmOmpMPIfxc: 
	${MPICC} -o $@ $@.c ${MODULO} ${FLAGS}

# Regla para compilar el programa mxmOmpMPIfxt
# Igual que la anterior, pero para el segundo ejecutable
mxmOmpMPIfxt: 
	${MPICC} -o $@ $@.c ${MODULO} ${FLAGS}

# Regla para limpiar los archivos compilados
# $(RM) elimina los ejecutables generados
clean:
	$(RM) ${PROGRAMAS}
