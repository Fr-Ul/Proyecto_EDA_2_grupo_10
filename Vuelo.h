#ifndef VUELO_H
#define VUELO_H

#include <string>
#include "MaxHeap.h"

struct Vuelo {
    std::string codigo, origen, destino, fecha, hora;
    int capacidad;
    int ocupados;        // reservas Confirmada + Pendiente + CheckIn
    MaxHeap espera;      // lista de espera propia del vuelo

    Vuelo() : capacidad(0), ocupados(0) {}
};

#endif
