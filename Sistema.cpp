#include "Sistema.h"
#include <fstream>
#include <iostream>
using namespace std;

// ---------- utilidades de lectura de CSV (sin STL de contenedores) ----------

static string limpiar(string s) {
    while (!s.empty() && (s[s.size() - 1] == '\r' || s[s.size() - 1] == '\n' ||
                          s[s.size() - 1] == ' '))
        s.erase(s.size() - 1);
    size_t i = 0;
    while (i < s.size() && s[i] == ' ') i++;
    return s.substr(i);
}

// Divide por comas en un arreglo de tamano fijo. El ultimo campo toma el resto.
static int dividir(const string& linea, string campos[], int max) {
    int n = 0;
    size_t ini = 0;
    while (n < max) {
        size_t p = (n == max - 1) ? string::npos : linea.find(',', ini);
        if (p == string::npos) { campos[n++] = limpiar(linea.substr(ini)); break; }
        campos[n++] = limpiar(linea.substr(ini, p - ini));
        ini = p + 1;
    }
    return n;
}

// ---------- construccion y vuelos ----------

Sistema::Sistema(int tamHash)
    : numVuelos(0), hash(tamHash), contadorLlegada(0) {}

int Sistema::buscarVuelo(const string& codigo) const {
    for (int i = 0; i < numVuelos; i++)
        if (vuelos[i].codigo == codigo) return i;
    return -1;
}

int Sistema::cargarVuelos(const string& ruta) {
    ifstream f(ruta.c_str());
    if (!f) { cout << "No se pudo abrir " << ruta << "\n"; return -1; }
    string linea, c[6];
    int leidos = 0;
    while (getline(f, linea)) {
        linea = limpiar(linea);
        if (linea.empty()) continue;
        if (dividir(linea, c, 6) < 6) continue;
        if (c[0] == "codigoVuelo") continue;                     // cabecera opcional
        if (numVuelos >= MAX_VUELOS || buscarVuelo(c[0]) >= 0) continue;
        int cap = 0;
        try { cap = stoi(c[5]); } catch (...) { continue; }
        if (cap <= 0) continue;
        Vuelo& v = vuelos[numVuelos++];
        v.codigo = c[0]; v.origen = c[1]; v.destino = c[2];
        v.fecha = c[3];  v.hora = c[4];   v.capacidad = cap;
        v.ocupados = 0;
        leidos++;
    }
    return leidos;
}

// ---------- 1. carga masiva ----------

Sistema::ResumenCarga Sistema::cargarReservas(const string& ruta) {
    ResumenCarga rc = {0, 0, 0, 0, 0, 0, 0};
    ifstream f(ruta.c_str());
    if (!f) { cout << "No se pudo abrir " << ruta << "\n"; return rc; }

    string linea, c[12];
    while (getline(f, linea)) {
        linea = limpiar(linea);
        if (linea.empty()) continue;
        if (dividir(linea, c, 12) < 12) { rc.rechazadas++; continue; }
        if (c[0] == "idReserva") continue;                       // cabecera opcional
        rc.leidas++;

        Clase cl = C_ECONOMICA; Categoria ca = CAT_REGULAR; Estado es = PENDIENTE;
        double precio = 0;
        if (!parseClase(c[8], cl) || !parseCategoria(c[9], ca) || !parseEstado(c[11], es)) {
            rc.rechazadas++; continue;
        }
        try { precio = stod(c[10]); } catch (...) { rc.rechazadas++; continue; }

        int v = buscarVuelo(c[3]);
        if (v < 0 || hash.buscar(c[0])) { rc.rechazadas++; continue; }   // vuelo inexistente o ID repetido

        Reserva* r = new Reserva();
        r->id = c[0]; r->dni = c[1]; r->pasajero = c[2]; r->codigoVuelo = c[3];
        r->origen = c[4]; r->destino = c[5]; r->fecha = c[6]; r->hora = c[7];
        r->clase = cl; r->categoria = ca; r->precio = precio; r->estado = es;
        r->idxVuelo = v;

        Vuelo& vu = vuelos[v];
        // El vuelo es la fuente de verdad de origen/destino/fecha/hora.
        if (r->origen != vu.origen || r->destino != vu.destino ||
            r->fecha != vu.fecha || r->hora != vu.hora) {
            r->origen = vu.origen; r->destino = vu.destino;
            r->fecha = vu.fecha;   r->hora = vu.hora;
            rc.corregidas++;
        }
        // Si el archivo trae mas reservas con asiento que la capacidad, el exceso pasa a espera.
        if (ocupaAsiento(r->estado)) {
            if (vu.ocupados >= vu.capacidad) { r->estado = EN_ESPERA; rc.reubicadas++; }
            else vu.ocupados++;
        }
        if (r->estado == EN_ESPERA) {
            r->llegada = ++contadorLlegada;      // el orden del archivo = orden de llegada
            vu.espera.insertar(r);
            rc.enHeaps++;
        }
        if (hash.insertar(r))  rc.enHash++;
        if (arbol.insertar(r)) rc.enArbol++;
    }

    cout << "Reservas leidas:        " << rc.leidas << "\n"
         << "Insertadas en la hash:  " << rc.enHash << "\n"
         << "Insertadas en el arbol: " << rc.enArbol << "\n"
         << "Insertadas en heaps:    " << rc.enHeaps << "\n"
         << "Rechazadas:             " << rc.rechazadas
         << " | corregidas con datos del vuelo: " << rc.corregidas
         << " | pasadas a espera por falta de cupo: " << rc.reubicadas << "\n";
    mostrarHeaps();
    return rc;
}

void Sistema::mostrarHeaps() const {
    cout << "Listas de espera (heaps con pasajeros):\n";
    int cuantos = 0;
    for (int i = 0; i < numVuelos; i++) {
        if (vuelos[i].espera.tamano() > 0) {
            cout << "  " << vuelos[i].codigo << ": " << vuelos[i].espera.tamano()
                 << " en espera (capacidad " << vuelos[i].capacidad << ")\n";
            cuantos++;
        }
    }
    if (cuantos == 0) cout << "  (ninguna)\n";
}

// ---------- 2. registro ----------

string Sistema::registrar(const Reserva& datos, Estado preferido, Reserva** creada) {
    if (creada) *creada = 0;
    if (datos.id.empty()) return "El idReserva no puede estar vacio.";
    if (datos.precio < 0) return "El precio no puede ser negativo.";
    if (hash.buscar(datos.id)) return "El idReserva ya existe.";
    int v = buscarVuelo(datos.codigoVuelo);
    if (v < 0) return "El vuelo no existe.";

    Reserva* r = new Reserva(datos);
    r->idxVuelo = v;
    Vuelo& vu = vuelos[v];
    r->origen = vu.origen; r->destino = vu.destino;
    r->fecha = vu.fecha;   r->hora = vu.hora;

    if (vu.ocupados < vu.capacidad) {
        r->estado = (preferido == CONFIRMADA) ? CONFIRMADA : PENDIENTE;
        vu.ocupados++;
    } else {
        r->estado = EN_ESPERA;
        r->llegada = ++contadorLlegada;
        vu.espera.insertar(r);
    }
    hash.insertar(r);
    arbol.insertar(r);
    if (creada) *creada = r;
    return "";
}

// ---------- 4 y 5. consultas sobre el arbol ----------

void Sistema::listarCronologico(int k) const {
    cout << "Primeras " << k << " reservas por fecha y hora de salida:\n";
    int m = arbol.listarPrimeras(k, true);
    cout << "(" << m << " mostradas)\n";
}

void Sistema::consultaRango(const string& fi, const string& ff) {
    cout << "Reservas con salida entre " << fi << " y " << ff << " (muestra de 10):\n";
    int total = arbol.consultaRango(fi, ff, 10, true);
    cout << "Encontradas: " << total << " | nodos visitados: " << arbol.nodosVisitados()
         << " de " << arbol.cantidad() << " en el arbol\n";
}

// ---------- 6. actualizacion de estado ----------

string Sistema::actualizarEstado(const string& id, Estado nuevo) {
    Reserva* r = hash.buscar(id);
    if (!r) return "No existe la reserva.";
    if (r->estado == CANCELADA) return "Una reserva Cancelada no se puede reactivar.";
    if (r->estado == PENDIENTE && nuevo == CONFIRMADA) { r->estado = nuevo; return ""; }
    if (r->estado == CONFIRMADA && nuevo == CHECKIN)   { r->estado = nuevo; return ""; }
    return "Transicion no permitida (solo Pendiente->Confirmada y Confirmada->CheckIn).";
}

// ---------- 7. cancelacion ----------

string Sistema::cancelar(const string& id, string& detalle) {
    detalle = "";
    Reserva* r = hash.buscar(id);
    if (!r) return "No existe la reserva.";
    if (r->estado == CANCELADA) return "La reserva ya esta cancelada.";

    Vuelo& v = vuelos[r->idxVuelo];
    if (r->estado == EN_ESPERA) {                      // Caso B: sale del heap desde cualquier posicion
        v.espera.eliminar(r);
        r->estado = CANCELADA;
        detalle = "Reserva en espera cancelada y retirada de la lista de espera de " + v.codigo + ".";
    } else {                                           // Caso A: libera asiento y promueve
        r->estado = CANCELADA;
        v.ocupados--;
        detalle = "Reserva cancelada; asiento liberado en " + v.codigo + ".";
        if (!v.espera.vacio()) {
            Reserva* p = v.espera.extraerMax();
            p->estado = CONFIRMADA;
            v.ocupados++;
            detalle += " Promovido: " + p->id + " (" + p->pasajero + ", " +
                       categoriaStr(p->categoria) + ") -> Confirmada.";
        } else {
            detalle += " La lista de espera estaba vacia.";
        }
    }
    return "";
}

// ---------- 8. eliminacion fisica ----------

string Sistema::eliminarFisica(const string& id) {
    string clave = id;                                 // copia: la Reserva se liberara
    Reserva* r = hash.buscar(clave);
    if (!r) return "No existe la reserva.";
    if (r->estado != CANCELADA) return "Solo se pueden eliminar reservas Canceladas.";
    arbol.eliminar(r);                                 // primero el arbol (usa la clave de r)
    hash.eliminar(clave);                              // la hash libera la Reserva
    return "";
}

// ---------- 9. lista de espera ----------

void Sistema::listaEspera(const string& codigoVuelo) const {
    int v = buscarVuelo(codigoVuelo);
    if (v < 0) { cout << "El vuelo no existe.\n"; return; }
    const Vuelo& vu = vuelos[v];
    cout << "Vuelo " << vu.codigo << " " << vu.origen << "->" << vu.destino << " "
         << vu.fecha << " " << vu.hora << " | ocupados " << vu.ocupados << "/"
         << vu.capacidad << " | en espera: " << vu.espera.tamano() << "\n";
    if (vu.espera.vacio()) { cout << "  (lista de espera vacia)\n"; return; }
    cout << "Siguiente a atender:\n";
    imprimirReserva(vu.espera.verMax());
    cout << "Lista completa en orden de prioridad:\n";
    vu.espera.imprimirOrdenado();
}

// ---------- 10. estadisticas ----------

void Sistema::estadisticas() const {
    int total = 0;
    double ingresos = 0, suma = 0;
    int porClase[3] = {0, 0, 0};
    int porEstado[5] = {0, 0, 0, 0, 0};
    string destinos[MAX_VUELOS];
    int cuentaDest[MAX_VUELOS];
    int nd = 0;
    const Reserva* mayor = 0;
    const Reserva* menor = 0;

    for (int i = 0; i < hash.tamanoTabla(); i++) {
        for (NodoHash* p = hash.cabeza(i); p; p = p->sig) {
            const Reserva* r = p->dato;
            total++;
            suma += r->precio;
            if (r->estado == CONFIRMADA || r->estado == CHECKIN) ingresos += r->precio;
            porClase[r->clase]++;
            porEstado[r->estado]++;
            int k = 0;
            while (k < nd && destinos[k] != r->destino) k++;
            if (k == nd && nd < MAX_VUELOS) { destinos[nd] = r->destino; cuentaDest[nd] = 0; nd++; }
            if (k < nd) cuentaDest[k]++;
            if (!mayor || r->precio > mayor->precio) mayor = r;
            if (!menor || r->precio < menor->precio) menor = r;
        }
    }
    if (total == 0) { cout << "No hay reservas cargadas.\n"; return; }

    cout << "Total de reservas: " << total << "\n";
    cout << "Ingresos (Confirmada + CheckIn): " << ingresos << "\n";
    cout << "Precio promedio (todas las reservas): " << suma / total << "\n";
    cout << "Por destino:\n";
    for (int k = 0; k < nd; k++) cout << "  " << destinos[k] << ": " << cuentaDest[k] << "\n";
    cout << "Por clase:  Economica " << porClase[C_ECONOMICA] << " | Premium " << porClase[C_PREMIUM]
         << " | Ejecutiva " << porClase[C_EJECUTIVA] << "\n";
    cout << "Por estado: ";
    for (int e = 0; e < 5; e++) cout << estadoStr((Estado)e) << " " << porEstado[e] << (e < 4 ? " | " : "\n");
    cout << "Reserva de mayor precio:\n"; imprimirReserva(mayor);
    cout << "Reserva de menor precio:\n"; imprimirReserva(menor);

    int mejor = -1;
    for (int i = 0; i < numVuelos; i++)
        if (vuelos[i].espera.tamano() > 0 &&
            (mejor < 0 || vuelos[i].espera.tamano() > vuelos[mejor].espera.tamano()))
            mejor = i;
    if (mejor < 0) cout << "Vuelo con mas pasajeros en espera: (ninguno)\n";
    else cout << "Vuelo con mas pasajeros en espera: " << vuelos[mejor].codigo << " ("
              << vuelos[mejor].espera.tamano() << ")\n";
}

// ---------- metricas y consistencia ----------

void Sistema::metricas() const {
    cout << "Factor de carga de la hash: " << hash.factorCarga()
         << " (" << hash.cantidad() << " / " << hash.tamanoTabla() << ")\n";
    cout << "Cadena mas larga de la hash: " << hash.cadenaMasLarga() << "\n";
    cout << "Altura del arbol AVL: " << arbol.altura() << " con " << arbol.cantidad() << " nodos\n";
    verificarConsistencia(true);
}

bool Sistema::verificarConsistencia(bool imprimir) const {
    bool ok = true;
    int nv = numVuelos > 0 ? numVuelos : 1;
    int* ocup = new int[nv]();
    int* esp = new int[nv]();
    int sinArbol = 0, sinHeap = 0;

    for (int i = 0; i < hash.tamanoTabla(); i++) {
        for (NodoHash* p = hash.cabeza(i); p; p = p->sig) {
            Reserva* r = p->dato;
            if (!arbol.contiene(r)) { sinArbol++; ok = false; }
            if (ocupaAsiento(r->estado)) ocup[r->idxVuelo]++;
            if (r->estado == EN_ESPERA) {
                esp[r->idxVuelo]++;
                if (!vuelos[r->idxVuelo].espera.contiene(r)) { sinHeap++; ok = false; }
            }
        }
    }
    int malVuelos = 0;
    for (int v = 0; v < numVuelos; v++) {
        if (ocup[v] != vuelos[v].ocupados || esp[v] != vuelos[v].espera.tamano() ||
            vuelos[v].ocupados > vuelos[v].capacidad || !vuelos[v].espera.esValido()) {
            malVuelos++; ok = false;
        }
    }
    if (hash.cantidad() != arbol.cantidad()) ok = false;

    if (imprimir) {
        cout << "Consistencia: hash=" << hash.cantidad() << " arbol=" << arbol.cantidad()
             << " | sin arbol: " << sinArbol << " | EnEspera fuera del heap: " << sinHeap
             << " | vuelos con descuadre: " << malVuelos << " -> " << (ok ? "OK" : "ERROR") << "\n";
    }
    delete[] ocup;
    delete[] esp;
    return ok;
}
