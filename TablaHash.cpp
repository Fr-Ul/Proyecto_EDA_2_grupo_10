#include "TablaHash.h"

TablaHash::TablaHash(int tamano) : tam(tamano), n(0) {
    tabla = new NodoHash*[tam];
    for (int i = 0; i < tam; i++) tabla[i] = 0;
}

TablaHash::~TablaHash() {
    for (int i = 0; i < tam; i++) {
        NodoHash* p = tabla[i];
        while (p) {
            NodoHash* sig = p->sig;
            delete p->dato;
            delete p;
            p = sig;
        }
    }
    delete[] tabla;
}

// Hash polinomial: h = h*131 + c (aritmetica sin signo), modulo el tamano (primo).
int TablaHash::indice(const std::string& id) const {
    unsigned long long h = 0;
    for (size_t i = 0; i < id.size(); i++)
        h = h * 131ULL + (unsigned char)id[i];
    return (int)(h % (unsigned long long)tam);
}

bool TablaHash::insertar(Reserva* r) {
    if (buscar(r->id)) return false;           // el ID no puede repetirse
    int i = indice(r->id);
    NodoHash* nuevo = new NodoHash(r);
    nuevo->sig = tabla[i];                      // insercion al inicio: O(1)
    tabla[i] = nuevo;
    n++;
    return true;
}

Reserva* TablaHash::buscar(const std::string& id) const {
    for (NodoHash* p = tabla[indice(id)]; p; p = p->sig)
        if (p->dato->id == id) return p->dato;
    return 0;
}

bool TablaHash::eliminar(const std::string& id) {
    int i = indice(id);
    NodoHash* ant = 0;
    for (NodoHash* p = tabla[i]; p; ant = p, p = p->sig) {
        if (p->dato->id == id) {
            if (ant) ant->sig = p->sig; else tabla[i] = p->sig;
            delete p->dato;
            delete p;
            n--;
            return true;
        }
    }
    return false;
}

int TablaHash::longitudCadena(int i) const {
    int c = 0;
    for (NodoHash* p = tabla[i]; p; p = p->sig) c++;
    return c;
}

int TablaHash::cadenaMasLarga() const {
    int mx = 0;
    for (int i = 0; i < tam; i++) {
        int c = longitudCadena(i);
        if (c > mx) mx = c;
    }
    return mx;
}
