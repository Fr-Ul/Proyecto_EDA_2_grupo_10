#ifndef MAXHEAP_H
#define MAXHEAP_H

#include "Reserva.h"

// Max-heap en arreglo dinamico (new[]/delete[]). Lista de espera de UN vuelo.
// Prioridad: categoria (Premium > Frecuente > Regular); en empate, gana el que
// llego antes (menor valor de 'llegada').
class MaxHeap {
private:
    Reserva** a;
    int n;
    int cap;

    MaxHeap(const MaxHeap&) = delete;
    MaxHeap& operator=(const MaxHeap&) = delete;

    static bool mayor(const Reserva* x, const Reserva* y);
    static void subirEn(Reserva** arr, int i);
    static void bajarEn(Reserva** arr, int tam, int i);
    void crecer();

public:
    explicit MaxHeap(int capInicial = 8);
    ~MaxHeap();

    void insertar(Reserva* r);                 // O(log m)
    Reserva* verMax() const { return n > 0 ? a[0] : 0; }
    Reserva* extraerMax();                     // O(log m)
    bool eliminar(const Reserva* r);           // busqueda secuencial O(m) + reordenar O(log m)
    bool contiene(const Reserva* r) const;

    int tamano() const { return n; }
    bool vacio() const { return n == 0; }

    void imprimirOrdenado() const;             // trabaja sobre una COPIA del arreglo
    bool esValido() const;                     // verifica la propiedad de heap
};

#endif
