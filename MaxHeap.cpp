#include "MaxHeap.h"
#include <iostream>
using namespace std;

MaxHeap::MaxHeap(int capInicial) : n(0), cap(capInicial > 0 ? capInicial : 8) {
    a = new Reserva*[cap];
}

MaxHeap::~MaxHeap() { delete[] a; }   // no libera las Reservas: son de la tabla hash

bool MaxHeap::mayor(const Reserva* x, const Reserva* y) {
    if (x->categoria != y->categoria) return x->categoria > y->categoria;
    return x->llegada < y->llegada;
}

void MaxHeap::subirEn(Reserva** arr, int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!mayor(arr[i], arr[p])) break;
        Reserva* t = arr[i]; arr[i] = arr[p]; arr[p] = t;
        i = p;
    }
}

void MaxHeap::bajarEn(Reserva** arr, int tam, int i) {
    while (true) {
        int l = 2 * i + 1, r = l + 1, m = i;
        if (l < tam && mayor(arr[l], arr[m])) m = l;
        if (r < tam && mayor(arr[r], arr[m])) m = r;
        if (m == i) break;
        Reserva* t = arr[i]; arr[i] = arr[m]; arr[m] = t;
        i = m;
    }
}

void MaxHeap::crecer() {
    int nuevaCap = cap * 2;
    Reserva** b = new Reserva*[nuevaCap];
    for (int i = 0; i < n; i++) b[i] = a[i];
    delete[] a;
    a = b;
    cap = nuevaCap;
}

void MaxHeap::insertar(Reserva* r) {
    if (n == cap) crecer();
    a[n] = r;
    subirEn(a, n);
    n++;
}

Reserva* MaxHeap::extraerMax() {
    if (n == 0) return 0;
    Reserva* mx = a[0];
    n--;
    if (n > 0) {
        a[0] = a[n];
        bajarEn(a, n, 0);
    }
    return mx;
}

// Elimina de una posicion cualquiera: se reemplaza por el ultimo elemento y se
// reordena hacia arriba o hacia abajo segun corresponda.
bool MaxHeap::eliminar(const Reserva* r) {
    int i = -1;
    for (int k = 0; k < n; k++)
        if (a[k] == r) { i = k; break; }
    if (i < 0) return false;
    n--;
    if (i == n) return true;                   // era el ultimo
    a[i] = a[n];
    subirEn(a, i);
    bajarEn(a, n, i);
    return true;
}

bool MaxHeap::contiene(const Reserva* r) const {
    for (int k = 0; k < n; k++)
        if (a[k] == r) return true;
    return false;
}

void MaxHeap::imprimirOrdenado() const {
    if (n == 0) { cout << "  (lista de espera vacia)\n"; return; }
    Reserva** copia = new Reserva*[n];
    for (int i = 0; i < n; i++) copia[i] = a[i];
    int m = n, pos = 1;
    while (m > 0) {
        Reserva* r = copia[0];
        m--;
        if (m > 0) { copia[0] = copia[m]; bajarEn(copia, m, 0); }
        cout << "  " << pos++ << ". " << r->id << " | " << r->pasajero
             << " | " << categoriaStr(r->categoria)
             << " | llegada #" << r->llegada << "\n";
    }
    delete[] copia;
}

bool MaxHeap::esValido() const {
    for (int i = 1; i < n; i++)
        if (mayor(a[i], a[(i - 1) / 2])) return false;
    return true;
}
