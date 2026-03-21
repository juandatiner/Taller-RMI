# Multiplicación de Matrices con MPI y OpenMP

Proyecto desarrollado para la materia de Ingeniería de Sistemas en la Pontificia Universidad Javeriana. La idea es multiplicar matrices grandes de forma rápida usando varias máquinas al mismo tiempo.

**Autores:** Juan Carlos Santamaria Orjuela, Juan Felipe Gutiérrez Adarme, Juan Daza, Juan Rincón

---

## ¿De qué trata?

Este proyecto multiplica matrices cuadradas (NxN) aprovechando dos tipos de paralelismo:

- **MPI** se encarga de repartir el trabajo entre varias máquinas conectadas en red (un cluster).
- **OpenMP** hace que cada máquina use todos sus núcleos de procesador para ir más rápido.

Se implementaron dos versiones del algoritmo:

- **FxC (Filas por Columnas):** la forma clásica de multiplicar matrices.
- **FxT (Filas por Transpuesta):** una versión optimizada que transpone la matriz B antes de multiplicar, lo que mejora el uso de la memoria caché y hace que corra más rápido en la práctica.

El programa usa un modelo **maestro-trabajador**: un proceso (el master) se encarga de dividir las matrices y repartirlas, los demás (workers) hacen el cálculo y devuelven sus resultados.

## Qué necesitas

- **Ubuntu 22.04** (o similar) en todas las máquinas.
- **OpenMPI** instalado en todos los nodos:
  ```
  sudo apt install openmpi-bin openmpi-common libopenmpi-dev -y
  ```
- **GCC** con soporte para OpenMP (viene incluido normalmente).
- Acceso **SSH sin contraseña** entre las máquinas del cluster.
- Una carpeta compartida entre los nodos (nosotros usamos **NFS**).

## Estructura del proyecto

| Archivo | Descripción |
|---|---|
| `mxmOmpMPIfxc.c` | Programa principal — multiplicación clásica (filas por columnas) |
| `mxmOmpMPIfxt.c` | Programa principal — multiplicación optimizada (con transpuesta) |
| `moduloMPI.c` | Funciones auxiliares (inicialización, multiplicación, medición de tiempo, etc.) |
| `moduloMPI.h` | Cabecera con las declaraciones de las funciones |
| `Makefile` | Para compilar todo fácilmente |
| `hosts` | Archivo con los nodos del cluster |
| `script.sh` | Script que automatiza las pruebas de rendimiento |

## Cómo armar el cluster

Esto es un resumen rápido. Nosotros usamos 3 máquinas virtuales: 1 master y 2 workers.

### 1. Configurar los nombres de los nodos

En **todas** las máquinas, editar `/etc/hosts` y agregar las IPs de los nodos:

```
10.43.100.144 master
10.43.100.90  nodo1
10.43.99.128  nodo2
```

(Obviamente con las IPs de tus propias máquinas.)

### 2. Configurar SSH sin contraseña

Desde el master:

```bash
ssh-keygen -t rsa
ssh-copy-id estudiante@nodo1
ssh-copy-id estudiante@nodo2
ssh-copy-id estudiante@localhost
```

Esto permite que el master se conecte a los workers sin pedir contraseña cada vez.

### 3. Compartir la carpeta del proyecto con NFS

**En el master:**

```bash
sudo apt install nfs-kernel-server -y
mkdir -p ~/TallerEvaluacionMPI
```

Agregar en `/etc/exports`:

```
/home/estudiante/TallerEvaluacionMPI nodo1(rw,sync,no_subtree_check)
/home/estudiante/TallerEvaluacionMPI nodo2(rw,sync,no_subtree_check)
```

Aplicar cambios:

```bash
sudo exportfs -a
sudo systemctl restart nfs-kernel-server
chmod -R 777 /home/estudiante/TallerEvaluacionMPI
```

**En cada worker:**

```bash
sudo apt install nfs-common -y
mkdir -p ~/TallerEvaluacionMPI
sudo mount master:/home/estudiante/TallerEvaluacionMPI /home/estudiante/TallerEvaluacionMPI
```

### 4. Editar el archivo `hosts` del proyecto

El archivo `hosts` (el del proyecto, no el de `/etc/`) le dice a MPI cuántos slots tiene cada nodo. El nuestro quedó así:

```
master slots=4
nodo1 slots=4
nodo2 slots=4
```

Ajústalo según la cantidad de núcleos que tenga cada máquina.

## Cómo compilar

Desde la carpeta del proyecto:

```bash
make
```

Eso genera dos ejecutables: `mxmOmpMPIfxc` y `mxmOmpMPIfxt`.

Si necesitas recompilar desde cero:

```bash
make clean
make
```

## Cómo ejecutar

La forma general de ejecutar es:

```bash
mpirun -np <procesos> --hostfile hosts ./<ejecutable> <tamaño_matriz> <hilos>
```

**Ejemplos:**

```bash
# Ejecución secuencial (1 solo proceso, 2 hilos), matriz 1200x1200
mpirun -np 1 --hostfile hosts ./mxmOmpMPIfxc 1200 2

# Con 3 procesos (1 master + 2 workers), 2 hilos, versión clásica
mpirun -np 3 --hostfile hosts ./mxmOmpMPIfxc 1200 2

# Con 3 procesos, 4 hilos, versión con transpuesta
mpirun -np 3 --hostfile hosts ./mxmOmpMPIfxt 1200 4
```

**Cosas a tener en cuenta:**

- Si usas más de 1 proceso, el tamaño de la matriz tiene que ser divisible por la cantidad de workers (que es `np - 1`). Por ejemplo, con `np=3` hay 2 workers, así que N debe ser divisible por 2.
- Con `np=1` el programa corre en modo secuencial, sin comunicación MPI.
- La salida del programa es el tiempo de ejecución en microsegundos.

## Cómo correr las pruebas automatizadas

El script `script.sh` ejecuta ambos algoritmos con varias combinaciones de procesos, hilos y tamaños de matriz, repitiendo cada configuración 30 veces. Los resultados se guardan en `resultados.csv`.

```bash
chmod +x script.sh
./script.sh
```

El CSV generado tiene este formato:

```
Algoritmo,NP,Hilos,N,Tiempo
mxmOmpMPIfxc,2,2,1200,84532
```

Con esos datos se pueden calcular promedios, speedup, eficiencia, y hacer gráficas.

## Resultados que obtuvimos

Algunas cosas que encontramos durante las pruebas:

- **El paralelismo funciona bien con matrices grandes.** Con matrices pequeñas (100, 200), el costo de comunicación entre procesos es mayor que el beneficio, y la versión secuencial termina siendo más rápida.
- **La versión con transpuesta (FxT) es más rápida** que la clásica (FxC), con una reducción de aproximadamente el 40% en el tiempo. Esto se debe a que accede a la memoria de forma más ordenada.
- **Usar 3 procesos da la mejor mejora**, pasando de ~9.4s a ~3s para una matriz de 1200x1200 con el algoritmo FxC.
- **Más hilos ayudan, pero con rendimientos decrecientes.** Pasar de 1 a 2 hilos mejora bastante, pero de 2 a 4 la mejora es menor.

## Notas

- Si al ejecutar ves mensajes de "Authorization required, but no authorization protocol specified", es normal y no afecta la ejecución.
- El programa solo imprime las matrices si son menores a 13x13 (para no llenar la terminal).
- Recuerda que las máquinas del cluster deben estar encendidas y accesibles por red para que funcione.
