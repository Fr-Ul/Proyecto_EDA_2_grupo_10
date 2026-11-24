#ifndef ARBOLAVL_H
#define ARBOLAVL_H

#include <string>
#include "Reserva.h"

struct NodoAVL {
    Reserva* dato;       // el arbol NO es duena de la Reserva (la libera la tabla hash)
    NodoAVL* izq;
    NodoAVL* der;
    int altura;          // hoja = 1
    explicit NodoAVL(Reserva* d) : dato(d), izq(0), der(0), altura(1) {}
};

// Arbol AVL ordenado por la clave compuesta fecha + hora + idReserva.
class ArbolAVL {
private:
    NodoAVL* raiz;
    int n;
    long long visitados;   // nodos visitados en la ultima consulta por rango

    ArbolAVL(const ArbolAVL&) = delete;
    ArbolAVL& operator=(const ArbolAVL&) = delete;

    static int alt(const NodoAVL* x);
    static void actualizar(NodoAVL* x);
    static int balance(const NodoAVL* x);
    static NodoAVL* rotDer(NodoAVL* y);
    static NodoAVL* rotIzq(NodoAVL* x);
    static NodoAVL* reequilibrar(NodoAVL* x);
    static NodoAVL* minimo(NodoAVL* x);

    NodoAVL* insertar(NodoAVL* x, Reserva* r, bool& ok);
    NodoAVL* eliminar(NodoAVL* x, const Reserva* r, bool& ok);
    void destruir(NodoAVL* x);
    void enOrden(const NodoAVL* x, int& restante, bool imprimir) const;
    void rango(const NodoAVL* x, const std::string& fi, const std::string& ff,
               int& cuenta, int& muestra, bool imprimir);

public:
    ArbolAVL();
    ~ArbolAVL();

    bool insertar(Reserva* r);                 // O(log n)
    bool eliminar(const Reserva* r);           // O(log n)
    bool contiene(const Reserva* r) const;     // O(log n)

    // Recorrido en orden, se detiene tras k nodos. Devuelve cuantos visito.
    int listarPrimeras(int k, bool imprimir) const;

    // Cuenta las reservas con fi <= fecha <= ff descartando ramas fuera del rango.
    // Imprime hasta maxMuestra si imprimir = true. O(log n + k).
    int consultaRango(const std::string& fi, const std::string& ff,
                      int maxMuestra, bool imprimir);
    long long nodosVisitados() const { return visitados; }

    int altura() const { return alt(raiz); }
    int cantidad() const { return n; }
};

#endif
