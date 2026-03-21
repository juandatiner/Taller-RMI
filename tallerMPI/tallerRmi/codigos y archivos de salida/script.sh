#!/bin/bash

echo "Algoritmo,NP,Hilos,N,Tiempo" > resultados.csv

for alg in mxmOmpMPIfxc mxmOmpMPIfxt
do
  for np in 1 2 3
  do
    for hilos in 1 2 4
    do
      for N in 600 1200 1800
      do

        # Validar divisibilidad
        workers=$((np - 1))
        if [ "$np" -eq 1 ] || [ $((N % workers)) -eq 0 ]; then

          for i in {1..30}
          do
            tiempo=$(mpirun -np $np --hostfile hosts ./$alg $N $hilos | tail -n 1)
            echo "$alg,$np,$hilos,$N,$tiempo" >> resultados.csv
          done

        fi

      done
    done
  done
done