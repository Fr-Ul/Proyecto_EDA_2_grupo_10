// Generador de datos de prueba (NO forma parte del sistema, por eso puede usar STL).
// Produce: vuelos.csv, reservas_masivo.csv, vuelos_pequeno.csv, reservas_pequeno.csv
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

static mt19937 rng(20261101);                       // semilla fija: datos reproducibles
static int rnd(int a, int b) { return uniform_int_distribution<int>(a, b)(rng); }
static string dos(int n) { ostringstream o; o << setw(2) << setfill('0') << n; return o.str(); }

struct V { string cod, org, dst, fecha, hora; int cap; };
struct R { string dni, nombre, cod, org, dst, fecha, hora, clase, cat, estado; double precio; };

static const char* CIUDADES[] = {"Lima", "Cusco", "Arequipa", "Piura", "Trujillo",
                                 "Iquitos", "Chiclayo", "Tarapoto", "Juliaca", "Pucallpa"};
static const char* HORAS[] = {"05:30", "06:45", "08:10", "09:20", "11:00",
                              "13:15", "15:40", "18:05", "20:30", "22:10"};
static const char* NOMBRES[] = {"Ana", "Luis", "Maria", "Carlos", "Rosa", "Jorge", "Lucia", "Pedro",
                                "Carmen", "Miguel", "Sofia", "Diego", "Elena", "Raul", "Valeria", "Andres"};
static const char* APELLIDOS[] = {"Torres", "Ramos", "Soto", "Quispe", "Flores", "Vargas", "Rojas", "Mendoza",
                                  "Castillo", "Huaman", "Salazar", "Paredes", "Cruz", "Diaz", "Chavez", "Ortiz"};
static const char* PREFIJOS[] = {"LA", "AV", "H2", "JA"};

// 90 dias: diciembre 2026, enero 2027, febrero 2027
static string fechaDesde(int d) {
    if (d < 31) return "2026-12-" + dos(d + 1);
    d -= 31;
    if (d < 31) return "2027-01-" + dos(d + 1);
    d -= 31;
    return "2027-02-" + dos(d + 1);
}

static R crear(const V& v, const string& estado, const string& catForzada = "") {
    R r;
    r.dni = to_string(rnd(10000000, 79999999));
    r.nombre = string(NOMBRES[rnd(0, 15)]) + " " + APELLIDOS[rnd(0, 15)];
    r.cod = v.cod; r.org = v.org; r.dst = v.dst; r.fecha = v.fecha; r.hora = v.hora;
    r.estado = estado;
    int x = rnd(1, 100);
    if (x <= 70)      { r.clase = "Economica"; r.precio = rnd(9000, 30000) / 100.0; }
    else if (x <= 90) { r.clase = "Premium";   r.precio = rnd(25000, 50000) / 100.0; }
    else              { r.clase = "Ejecutiva"; r.precio = rnd(45000, 120000) / 100.0; }
    if (!catForzada.empty()) r.cat = catForzada;
    else {
        int y = rnd(1, 100);
        r.cat = y <= 65 ? "Regular" : (y <= 90 ? "Frecuente" : "Premium");
    }
    return r;
}

static string estadoOcupa() {
    int x = rnd(1, 100);
    return x <= 60 ? "Confirmada" : (x <= 90 ? "Pendiente" : "CheckIn");
}

static string catMixta(int k) {            // las primeras de cada espera: categorias mezcladas
    return k == 0 ? "Premium" : (k == 1 ? "Frecuente" : (k == 2 ? "Regular" : ""));
}

static void escribirVuelos(const string& ruta, const vector<V>& vs) {
    ofstream f(ruta.c_str());
    for (size_t i = 0; i < vs.size(); i++)
        f << vs[i].cod << "," << vs[i].org << "," << vs[i].dst << ","
          << vs[i].fecha << "," << vs[i].hora << "," << vs[i].cap << "\n";
}

static void escribirReservas(const string& ruta, vector<R>& rs) {
    shuffle(rs.begin(), rs.end(), rng);       // mezcla el orden; los IDs se asignan despues
    ofstream f(ruta.c_str());
    f << fixed << setprecision(2);
    for (size_t i = 0; i < rs.size(); i++) {
        ostringstream id;
        id << "RES-" << setw(6) << setfill('0') << (i + 1);
        f << id.str() << "," << rs[i].dni << "," << rs[i].nombre << "," << rs[i].cod << ","
          << rs[i].org << "," << rs[i].dst << "," << rs[i].fecha << "," << rs[i].hora << ","
          << rs[i].clase << "," << rs[i].cat << "," << rs[i].precio << "," << rs[i].estado << "\n";
    }
}

int main() {
    // ---------------- MASIVO: 50 vuelos, 10,000 reservas ----------------
    vector<V> vs;
    for (int i = 0; i < 50; i++) {
        V v;
        v.cod = string(PREFIJOS[i % 4]) + to_string(100 + i * 13);   // codigos unicos
        int o = rnd(0, 9), d = rnd(0, 9);
        while (d == o) d = rnd(0, 9);
        v.org = CIUDADES[o]; v.dst = CIUDADES[d];
        v.fecha = fechaDesde(rnd(0, 89));
        v.hora = HORAS[rnd(0, 9)];
        v.cap = rnd(150, 300);
        vs.push_back(v);
    }
    // Varios vuelos con la misma fecha y hora de salida (prueba del desempate por ID en el arbol)
    vs[1].fecha = vs[0].fecha; vs[1].hora = vs[0].hora;
    vs[2].fecha = vs[0].fecha; vs[2].hora = vs[0].hora;
    escribirVuelos("vuelos.csv", vs);

    vector<R> rs;
    vector<int> ocup(vs.size(), 0);
    // 6 vuelos llenos (indices 0..5), cada uno con 6-12 pasajeros en espera de categorias mezcladas
    for (int i = 0; i < 6; i++) {
        for (int k = 0; k < vs[i].cap; k++) rs.push_back(crear(vs[i], estadoOcupa()));
        ocup[i] = vs[i].cap;
        int w = rnd(6, 12);
        for (int k = 0; k < w; k++) rs.push_back(crear(vs[i], "EnEspera", catMixta(k)));
    }
    // El resto de reservas se reparte en los demas vuelos sin superar su capacidad
    vector<int> libres;
    for (int i = 6; i < (int)vs.size(); i++) libres.push_back(i);
    while (rs.size() < 10000) {
        if (libres.empty()) { rs.push_back(crear(vs[rnd(0, (int)vs.size() - 1)], "Cancelada")); continue; }
        int p = rnd(0, (int)libres.size() - 1);
        int i = libres[p];
        if (rnd(1, 100) <= 4) { rs.push_back(crear(vs[i], "Cancelada")); continue; }
        rs.push_back(crear(vs[i], estadoOcupa()));
        if (++ocup[i] >= vs[i].cap) { libres[p] = libres.back(); libres.pop_back(); }
    }
    escribirReservas("reservas_masivo.csv", rs);

    // ---------------- PEQUENO: 6 vuelos de capacidad baja, 45 reservas ----------------
    // (con capacidades de 150-300 una prueba manual de lista de espera seria imposible)
    vector<V> sv;
    V a1 = {"PQ001", "Lima", "Cusco", "2026-12-01", "06:45", 5};
    V a2 = {"PQ002", "Lima", "Arequipa", "2026-12-01", "06:45", 5};   // misma fecha y hora que PQ001
    V a3 = {"PQ003", "Lima", "Piura", "2026-12-02", "09:20", 5};
    V a4 = {"PQ004", "Cusco", "Lima", "2026-12-05", "11:00", 10};
    V a5 = {"PQ005", "Arequipa", "Lima", "2026-12-12", "15:40", 10};
    V a6 = {"PQ006", "Lima", "Iquitos", "2026-12-20", "18:05", 10};
    sv.push_back(a1); sv.push_back(a2); sv.push_back(a3);
    sv.push_back(a4); sv.push_back(a5); sv.push_back(a6);
    escribirVuelos("vuelos_pequeno.csv", sv);

    vector<R> sr;
    int espera[3] = {6, 5, 7};
    for (int i = 0; i < 3; i++) {                      // 3 vuelos llenos con espera
        for (int k = 0; k < 5; k++) sr.push_back(crear(sv[i], estadoOcupa()));
        for (int k = 0; k < espera[i]; k++) sr.push_back(crear(sv[i], "EnEspera", catMixta(k)));
    }
    for (int k = 0; k < 4; k++) sr.push_back(crear(sv[3], estadoOcupa()));
    sr.push_back(crear(sv[3], "Cancelada"));
    for (int k = 0; k < 3; k++) sr.push_back(crear(sv[4], estadoOcupa()));
    for (int k = 0; k < 4; k++) sr.push_back(crear(sv[5], estadoOcupa()));
    escribirReservas("reservas_pequeno.csv", sr);

    cout << "Generados: vuelos.csv (" << vs.size() << " vuelos), reservas_masivo.csv (" << rs.size()
         << " reservas),\n           vuelos_pequeno.csv (" << sv.size() << " vuelos), reservas_pequeno.csv ("
         << sr.size() << " reservas)\n";
    return 0;
}
