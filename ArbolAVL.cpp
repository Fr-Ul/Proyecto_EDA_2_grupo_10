#include "ArbolAVL.h"

ArbolAVL::ArbolAVL() : raiz(0), n(0), visitados(0) {}

ArbolAVL::~ArbolAVL() { destruir(raiz); }

void ArbolAVL::destruir(NodoAVL* x) {
    if (!x) return;
    destruir(x->izq);
    destruir(x->der);
    delete x;
}

int ArbolAVL::alt(const NodoAVL* x) { return x ? x->altura : 0; }

void ArbolAVL::actualizar(NodoAVL* x) {
    int a = alt(x->izq), b = alt(x->der);
    x->altura = 1 + (a > b ? a : b);
}

int ArbolAVL::balance(const NodoAVL* x) {
    return x ? alt(x->izq) - alt(x->der) : 0;
}

NodoAVL* ArbolAVL::rotDer(NodoAVL* y) {
    NodoAVL* x = y->izq;
    y->izq = x->der;
    x->der = y;
    actualizar(y);
    actualizar(x);
    return x;
}

NodoAVL* ArbolAVL::rotIzq(NodoAVL* x) {
    NodoAVL* y = x->der;
    x->der = y->izq;
    y->izq = x;
    actualizar(x);
    actualizar(y);
    return y;
}

NodoAVL* ArbolAVL::reequilibrar(NodoAVL* x) {
    actualizar(x);
    int b = balance(x);
    if (b > 1) {                                   // pesado a la izquierda
        if (balance(x->izq) < 0) x->izq = rotIzq(x->izq);   // caso izq-der
        return rotDer(x);
    }
    if (b < -1) {                                  // pesado a la derecha
        if (balance(x->der) > 0) x->der = rotDer(x->der);   // caso der-izq
        return rotIzq(x);
    }
    return x;
}

NodoAVL* ArbolAVL::minimo(NodoAVL* x) {
    while (x->izq) x = x->izq;
    return x;
}

NodoAVL* ArbolAVL::insertar(NodoAVL* x, Reserva* r, bool& ok) {
    if (!x) { ok = true; return new NodoAVL(r); }
    int c = compararClave(r, x->dato);
    if (c < 0) x->izq = insertar(x->izq, r, ok);
    else if (c > 0) x->der = insertar(x->der, r, ok);
    else { ok = false; return x; }                 // clave repetida
    return reequilibrar(x);
}

bool ArbolAVL::insertar(Reserva* r) {
    bool ok = false;
    raiz = insertar(raiz, r, ok);
    if (ok) n++;
    return ok;
}

NodoAVL* ArbolAVL::eliminar(NodoAVL* x, const Reserva* r, bool& ok) {
    if (!x) { ok = false; return 0; }
    int c = compararClave(r, x->dato);
    if (c < 0) x->izq = eliminar(x->izq, r, ok);
    else if (c > 0) x->der = eliminar(x->der, r, ok);
    else {
        ok = true;
        if (!x->izq || !x->der) {                  // 0 o 1 hijo
            NodoAVL* h = x->izq ? x->izq : x->der;
            delete x;
            return h;
        }
        NodoAVL* s = minimo(x->der);               // 2 hijos: sucesor en orden
        x->dato = s->dato;
        bool ok2 = false;
        x->der = eliminar(x->der, s->dato, ok2);
    }
    return reequilibrar(x);
}

bool ArbolAVL::eliminar(const Reserva* r) {
    bool ok = false;
    raiz = eliminar(raiz, r, ok);
    if (ok) n--;
    return ok;
}

bool ArbolAVL::contiene(const Reserva* r) const {
    const NodoAVL* x = raiz;
    while (x) {
        int c = compararClave(r, x->dato);
        if (c == 0) return x->dato == r;
        x = (c < 0) ? x->izq : x->der;
    }
    return false;
}

void ArbolAVL::enOrden(const NodoAVL* x, int& restante, bool imprimir) const {
    if (!x || restante <= 0) return;
    enOrden(x->izq, restante, imprimir);
    if (restante <= 0) return;
    if (imprimir) imprimirReserva(x->dato);
    restante--;
    enOrden(x->der, restante, imprimir);
}

int ArbolAVL::listarPrimeras(int k, bool imprimir) const {
    int restante = k;
    enOrden(raiz, restante, imprimir);
    return k - restante;
}

// Como el arbol esta ordenado por (fecha, hora, id), la fecha es monotona en el
// recorrido en orden: se puede podar sin mirar la hora ni el id.
void ArbolAVL::rango(const NodoAVL* x, const std::string& fi, const std::string& ff,
                     int& cuenta, int& muestra, bool imprimir) {
    if (!x) return;
    visitados++;
    if (x->dato->fecha < fi) {                     // este nodo y todo su subarbol izq quedan fuera
        rango(x->der, fi, ff, cuenta, muestra, imprimir);
        return;
    }
    if (x->dato->fecha > ff) {                     // este nodo y todo su subarbol der quedan fuera
        rango(x->izq, fi, ff, cuenta, muestra, imprimir);
        return;
    }
    rango(x->izq, fi, ff, cuenta, muestra, imprimir);
    cuenta++;
    if (imprimir && muestra > 0) { imprimirReserva(x->dato); muestra--; }
    rango(x->der, fi, ff, cuenta, muestra, imprimir);
}

int ArbolAVL::consultaRango(const std::string& fi, const std::string& ff,
                            int maxMuestra, bool imprimir) {
    int cuenta = 0, muestra = maxMuestra;
    visitados = 0;
    rango(raiz, fi, ff, cuenta, muestra, imprimir);
    return cuenta;
}
