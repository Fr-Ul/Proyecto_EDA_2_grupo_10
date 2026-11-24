#ifndef TABLAHASH_H
#define TABLAHASH_H

#include <string>
#include "Reserva.h"

struct NodoHash {
    Reserva* dato;
    NodoHash* sig;
    explicit NodoHash(Reserva* d) : dato(d), sig(0) {}
};

// Tabla hash con encadenamiento (listas enlazadas propias).
// Clave: idReserva. Es la DUENA de los objetos Reserva: el arbol y los heaps
// solo guardan punteros, asi que cambiar el estado aqui se ve en todas partes.
class TablaHash {
private:
    NodoHash** tabla;
    int tam;
    int n;

    TablaHash(const TablaHash&) = delete;
    TablaHash& operator=(const TablaHash&) = delete;

public:
    explicit TablaHash(int tamano = 20011);   // primo; con 10,000 datos -> factor ~0.5
    ~TablaHash();

    int indice(const std::string& id) const;
    bool insertar(Reserva* r);                 // false si el id ya existe
    Reserva* buscar(const std::string& id) const;
    bool eliminar(const std::string& id);      // libera el nodo Y la Reserva

    int cantidad() const { return n; }
    int tamanoTabla() const { return tam; }
    NodoHash* cabeza(int i) const { return tabla[i]; }
    double factorCarga() const { return (double)n / tam; }
    int cadenaMasLarga() const;
    int longitudCadena(int i) const;
};

#endif
