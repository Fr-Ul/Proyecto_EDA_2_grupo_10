#include <iostream>
#include <string>
#include <chrono>
#include "Sistema.h"
using namespace std;

static string leer(const string& msg) {
    cout << msg;
    string s;
    if (!getline(cin, s)) return "";
    while (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
    return s;
}

static string conDefecto(const string& msg, const string& def) {
    string s = leer(msg + " [" + def + "]: ");
    return s.empty() ? def : s;
}

static int leerEntero(const string& msg, int def) {
    string s = leer(msg);
    if (s.empty()) return def;
    try { return stoi(s); } catch (...) { return def; }
}

static void menu() {
    cout << "\n===== SISTEMA DE RESERVAS AEREAS =====\n"
         << " 1. Carga masiva (vuelos.csv y reservas.csv)\n"
         << " 2. Registrar nueva reserva\n"
         << " 3. Buscar reserva por ID\n"
         << " 4. Listado cronologico (primeras N)\n"
         << " 5. Consulta por rango de fechas\n"
         << " 6. Actualizar estado de una reserva\n"
         << " 7. Cancelar reserva\n"
         << " 8. Eliminacion fisica (solo Canceladas)\n"
         << " 9. Lista de espera de un vuelo\n"
         << "10. Estadisticas generales\n"
         << "11. Metricas de estructuras y verificacion de consistencia\n"
         << " 0. Salir\n";
}

int main(int argc, char** argv) {
    cout.setf(ios::fixed);
    cout.precision(2);

    Sistema sis;
    if (argc >= 3) {                                   // ./reservas vuelos.csv reservas.csv
        if (sis.cargarVuelos(argv[1]) >= 0) sis.cargarReservas(argv[2]);
    }

    while (true) {
        menu();
        string linea = leer("Opcion: ");
        if (!cin) break;
        int op = -1;
        try { op = stoi(linea); } catch (...) { op = -1; }

        if (op == 0) break;

        switch (op) {
        case 1: {
            string fv = conDefecto("Archivo de vuelos", "vuelos.csv");
            string fr = conDefecto("Archivo de reservas", "reservas_masivo.csv");
            auto t0 = chrono::steady_clock::now();
            int nv = sis.cargarVuelos(fv);
            if (nv < 0) break;
            cout << "Vuelos cargados: " << nv << "\n";
            sis.cargarReservas(fr);
            auto t1 = chrono::steady_clock::now();
            cout << "Tiempo de carga: "
                 << chrono::duration<double, milli>(t1 - t0).count() << " ms\n";
            break;
        }
        case 2: {
            Reserva d;
            d.id = leer("idReserva: ");
            d.dni = leer("DNI: ");
            d.pasajero = leer("Pasajero: ");
            d.codigoVuelo = leer("Codigo de vuelo: ");
            if (!parseClase(leer("Clase (Economica/Premium/Ejecutiva): "), d.clase)) {
                cout << "Clase invalida.\n"; break;
            }
            if (!parseCategoria(leer("Categoria (Premium/Frecuente/Regular): "), d.categoria)) {
                cout << "Categoria invalida.\n"; break;
            }
            try { d.precio = stod(leer("Precio: ")); } catch (...) { cout << "Precio invalido.\n"; break; }
            string pref = conDefecto("Estado si hay asiento (Confirmada/Pendiente)", "Pendiente");
            Reserva* creada = 0;
            string err = sis.registrar(d, pref == "Confirmada" ? CONFIRMADA : PENDIENTE, &creada);
            if (!err.empty()) cout << "Error: " << err << "\n";
            else {
                cout << "Reserva registrada:\n";
                imprimirReserva(creada);
                if (creada->estado == EN_ESPERA) cout << "El vuelo esta lleno: ingreso a la lista de espera.\n";
            }
            break;
        }
        case 3: {
            Reserva* r = sis.buscar(leer("idReserva: "));
            if (r) imprimirReserva(r); else cout << "No existe una reserva con ese ID.\n";
            break;
        }
        case 4:
            sis.listarCronologico(leerEntero("Cuantas mostrar [20]: ", 20));
            break;
        case 5: {
            string fi = leer("Fecha inicial (AAAA-MM-DD): ");
            string ff = leer("Fecha final   (AAAA-MM-DD): ");
            sis.consultaRango(fi, ff);
            break;
        }
        case 6: {
            string id = leer("idReserva: ");
            Estado nuevo;
            if (!parseEstado(leer("Nuevo estado (Confirmada/CheckIn): "), nuevo)) {
                cout << "Estado invalido.\n"; break;
            }
            string err = sis.actualizarEstado(id, nuevo);
            if (err.empty()) { cout << "Estado actualizado:\n"; imprimirReserva(sis.buscar(id)); }
            else cout << "Error: " << err << "\n";
            break;
        }
        case 7: {
            string detalle;
            string err = sis.cancelar(leer("idReserva: "), detalle);
            cout << (err.empty() ? detalle : "Error: " + err) << "\n";
            break;
        }
        case 8: {
            string id = leer("idReserva: ");
            string err = sis.eliminarFisica(id);
            if (!err.empty()) { cout << "Error: " << err << "\n"; break; }
            cout << "Reserva eliminada. Verificacion: ";
            cout << (sis.buscar(id) ? "AUN APARECE (error)" : "ya no aparece en la busqueda por ID") << "\n";
            break;
        }
        case 9:
            sis.listaEspera(leer("Codigo de vuelo: "));
            break;
        case 10:
            sis.estadisticas();
            break;
        case 11:
            sis.metricas();
            break;
        default:
            cout << "Opcion invalida.\n";
        }
    }
    cout << "Hasta luego.\n";
    return 0;
}
