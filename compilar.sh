#!/bin/sh
set -e
SRC="Sistema.cpp TablaHash.cpp ArbolAVL.cpp MaxHeap.cpp"
g++ -std=c++11 -O2 -Wall -o reservas  main.cpp      $SRC
g++ -std=c++11 -O2 -Wall -o benchmark benchmark.cpp $SRC
g++ -std=c++11 -O2 -Wall -o generador generador.cpp
echo "Listo: ./generador, ./reservas, ./benchmark"
