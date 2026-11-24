// Genera las tablas de la seccion "Resultados" del informe.
// Uso: ./benchmark [vuelos.csv] [reservas_masivo.csv]
#include <iostream>
#include <string>
#include <chrono>
#include "Sistema.h"
using namespace std;

typedef chrono::steady_clock Reloj;

static double usMed(Reloj::time_point a, Reloj::time_point b) {
    return chrono::duration<double, std::micro>(b - a).count();
}

static void fila(const string& op, const string& datos, int reps, double totalUs) {
    cout << op << "\t" << datos << "\t" << reps << "\t" << totalUs / reps << " us\n";
}

int main(int argc, char** argv) {
    string fv = argc > 1 ? argv[1] : "vuelos.csv";
    string fr = argc > 2 ? argv[2] : "reservas_masivo.csv";
    cout.setf(ios::fixed);
    cout.precision(3);

    Sistema* sis = new Sistema();

    // ---- Carga masiva ----
    Reloj::time_point t0 = Reloj::now();
    if (sis->cargarVuelos(fv) < 0) return 1;
    Sistema::ResumenCarga rc = sis->cargarReservas(fr);
    Reloj::time_point t1 = Reloj::now();
    double cargaMs = usMed(t0, t1) / 1000.0;
    int n = sis->hash.cantidad();
    if (n == 0) return 1;
    // Metricas justo despues de la carga (antes de insertar/eliminar)
    double factorCarga = sis->hash.factorCarga();
    int cadenaMax = sis->hash.cadenaMasLarga();
    int alturaCarga = sis->arbol.altura();
    int nodosCarga = sis->arbol.cantidad();

    // Arreglo plano con todas las reservas (para elegir IDs y para la busqueda secuencial)
    Reserva** todas = new Reserva*[n];
    int m = 0;
    for (int i = 0; i < sis->hash.tamanoTabla(); i++)
        for (NodoHash* p = sis->hash.cabeza(i); p; p = p->sig) todas[m++] = p->dato;

    const int NB = 1000;
    string* ids = new string[NB];
    for (int i = 0; i < NB; i++) ids[i] = todas[(i * 7919) % n]->id;   // IDs repartidos
    int hits = 0;

    cout << "\n=== Tiempos (promedio por operacion) ===\n";
    cout << "Operacion\tDatos\tRepeticiones\tTiempo\n";
    cout << "Carga masiva\t" << n << " reservas\t1\t" << cargaMs << " ms\n";

    // ---- Busqueda por ID (hash) ----
    t0 = Reloj::now();
    for (int i = 0; i < NB; i++) if (sis->hash.buscar(ids[i])) hits++;
    t1 = Reloj::now();
    fila("Busqueda por ID (hash)", to_string(n) + " reservas", NB, usMed(t0, t1));

    // ---- Busqueda secuencial del mismo ID ----
    t0 = Reloj::now();
    for (int i = 0; i < NB; i++)
        for (int j = 0; j < n; j++)
            if (todas[j]->id == ids[i]) { hits++; break; }
    t1 = Reloj::now();
    fila("Busqueda secuencial", to_string(n) + " reservas", NB, usMed(t0, t1));

    // ---- Insercion individual (en el vuelo con mas asientos libres) ----
    int mejorV = 0;
    for (int v = 1; v < sis->numVuelos; v++)
        if (sis->vuelos[v].capacidad - sis->vuelos[v].ocupados >
            sis->vuelos[mejorV].capacidad - sis->vuelos[mejorV].ocupados) mejorV = v;
    t0 = Reloj::now();
    for (int i = 0; i < 100; i++) {
        Reserva d;
        d.id = "BEN-" + to_string(i);
        d.dni = "12345678"; d.pasajero = "Prueba Benchmark";
        d.codigoVuelo = sis->vuelos[mejorV].codigo;
        sis->registrar(d, PENDIENTE);
    }
    t1 = Reloj::now();
    fila("Insercion individual", to_string(n) + " reservas", 100, usMed(t0, t1));

    // ---- Consulta ordenada (primeras 50) ----
    int visitadas = 0;
    t0 = Reloj::now();
    for (int i = 0; i < 100; i++) visitadas += sis->arbol.listarPrimeras(50, false);
    t1 = Reloj::now();
    fila("Consulta ordenada (50)", to_string(n) + " reservas", 100, usMed(t0, t1));

    // ---- Consulta por rango (un mes) ----
    int enRango = 0;
    t0 = Reloj::now();
    for (int i = 0; i < 100; i++) enRango = sis->arbol.consultaRango("2026-12-01", "2026-12-31", 0, false);
    t1 = Reloj::now();
    fila("Consulta por rango (1 mes)", to_string(n) + " reservas", 100, usMed(t0, t1));
    long long visRango = sis->arbol.nodosVisitados();

    // ---- Actualizacion de estado: 10 reservas Pendiente -> Confirmada (una vez cada una) ----
    string idsAct[10]; int na = 0;
    for (int j = 0; j < n && na < 10; j++)
        if (todas[j]->estado == PENDIENTE) idsAct[na++] = todas[j]->id;
    t0 = Reloj::now();
    for (int i = 0; i < na; i++) sis->actualizarEstado(idsAct[i], CONFIRMADA);
    t1 = Reloj::now();
    fila("Actualizacion de estado", "10 reservas", na, usMed(t0, t1));

    // ---- Cancelacion con promocion: 10 reservas con asiento en vuelos con lista de espera ----
    int sel[Sistema::MAX_VUELOS];
    for (int v = 0; v < Sistema::MAX_VUELOS; v++) sel[v] = 0;
    string idsCan[10]; int nc = 0;
    for (int j = 0; j < n && nc < 10; j++) {
        Reserva* r = todas[j];
        if (ocupaAsiento(r->estado) && sel[r->idxVuelo] < sis->vuelos[r->idxVuelo].espera.tamano()) {
            idsCan[nc++] = r->id;
            sel[r->idxVuelo]++;
        }
    }
    string det;
    t0 = Reloj::now();
    for (int i = 0; i < nc; i++) sis->cancelar(idsCan[i], det);
    t1 = Reloj::now();
    fila("Cancelacion con promocion", "10 reservas", nc, usMed(t0, t1));

    // ---- Eliminacion fisica de esas 10 (ya estan Canceladas) ----
    t0 = Reloj::now();
    for (int i = 0; i < nc; i++) sis->eliminarFisica(idsCan[i]);
    t1 = Reloj::now();
    fila("Eliminacion fisica", "10 reservas", nc, usMed(t0, t1));
    bool borradas = true;
    for (int i = 0; i < nc; i++) if (sis->hash.buscar(idsCan[i])) borradas = false;

    // ---- Metricas de las estructuras ----
    cout << "\n=== Metricas ===\n";
    cout << "Factor de carga de la hash\t" << factorCarga << "\n";
    cout << "Cadena mas larga de la hash\t" << cadenaMax << "\n";
    cout << "Altura del arbol AVL (" << nodosCarga << " reservas)\t" << alturaCarga << "\n";
    cout << "Nodos visitados en la consulta de un mes\t" << visRango << " de " << sis->arbol.cantidad()
         << " (encontradas: " << enRango << ")\n";

    // ---- Casos extremos ----
    cout << "\n=== Casos extremos ===\n";
    {   // 60 IDs en la misma posicion de una hash pequena
        TablaHash th(101);
        Reserva* col[60];
        int c = 0;
        for (int k = 0; c < 60; k++) {
            string id = "COL-" + to_string(k);
            if (th.indice(id) == 0) {
                Reserva* r = new Reserva();
                r->id = id;
                th.insertar(r);
                col[c++] = r;
            }
        }
        bool todos = true;
        for (int i = 0; i < 60; i++) if (th.buscar(col[i]->id) != col[i]) todos = false;
        cout << "Colisiones: 60 IDs en la posicion 0 -> cadena mas larga " << th.cadenaMasLarga()
             << ", todos encontrados: " << (todos ? "SI" : "NO") << "\n";
    }
    {   // 1,000 reservas con fechas estrictamente crecientes
        ArbolAVL arb;
        Reserva* rs[1000];
        for (int i = 0; i < 1000; i++) {
            rs[i] = new Reserva();
            rs[i]->id = "ASC-" + to_string(i);
            int anio = 2030 + i / 336, mes = (i / 28) % 12 + 1, dia = i % 28 + 1;
            rs[i]->fecha = to_string(anio) + "-" + (mes < 10 ? "0" : "") + to_string(mes) + "-" +
                           (dia < 10 ? "0" : "") + to_string(dia);
            rs[i]->hora = "08:00";
            arb.insertar(rs[i]);
        }
        cout << "Orden desfavorable: 1000 inserciones crecientes -> altura " << arb.altura()
             << " (una lista degenerada tendria 1000)\n";
        for (int i = 0; i < 1000; i++) delete rs[i];
    }

    // ---- Consistencia final ----
    cout << "\n=== Verificaciones ===\n";
    cout << "Eliminadas ya no aparecen en la hash: " << (borradas ? "SI" : "NO") << "\n";
    sis->verificarConsistencia(true);

    if (hits < 0 || visitadas < 0) cout << "";       // evita que el compilador descarte los bucles
    (void)rc;
    delete[] todas;
    delete[] ids;
    delete sis;
    return 0;
}
