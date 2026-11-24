#ifndef RESERVA_H
#define RESERVA_H

#include <string>
#include <iostream>

enum Estado { CONFIRMADA, PENDIENTE, EN_ESPERA, CHECKIN, CANCELADA };
enum Clase { C_ECONOMICA, C_PREMIUM, C_EJECUTIVA };
// El valor numerico de la categoria ES su prioridad (mayor = mas prioridad).
enum Categoria { CAT_REGULAR = 0, CAT_FRECUENTE = 1, CAT_PREMIUM = 2 };

inline const char* estadoStr(Estado e) {
    switch (e) {
        case CONFIRMADA: return "Confirmada";
        case PENDIENTE:  return "Pendiente";
        case EN_ESPERA:  return "EnEspera";
        case CHECKIN:    return "CheckIn";
        default:         return "Cancelada";
    }
}
inline const char* claseStr(Clase c) {
    switch (c) {
        case C_ECONOMICA: return "Economica";
        case C_PREMIUM:   return "Premium";
        default:          return "Ejecutiva";
    }
}
inline const char* categoriaStr(Categoria c) {
    switch (c) {
        case CAT_REGULAR:   return "Regular";
        case CAT_FRECUENTE: return "Frecuente";
        default:            return "Premium";
    }
}

inline bool parseEstado(const std::string& s, Estado& e) {
    if (s == "Confirmada") e = CONFIRMADA;
    else if (s == "Pendiente") e = PENDIENTE;
    else if (s == "EnEspera") e = EN_ESPERA;
    else if (s == "CheckIn") e = CHECKIN;
    else if (s == "Cancelada") e = CANCELADA;
    else return false;
    return true;
}
inline bool parseClase(const std::string& s, Clase& c) {
    if (s == "Economica") c = C_ECONOMICA;
    else if (s == "Premium") c = C_PREMIUM;
    else if (s == "Ejecutiva") c = C_EJECUTIVA;
    else return false;
    return true;
}
inline bool parseCategoria(const std::string& s, Categoria& c) {
    if (s == "Regular") c = CAT_REGULAR;
    else if (s == "Frecuente") c = CAT_FRECUENTE;
    else if (s == "Premium") c = CAT_PREMIUM;
    else return false;
    return true;
}

// Una reserva ocupa asiento si esta Confirmada, Pendiente o CheckIn.
inline bool ocupaAsiento(Estado e) {
    return e == CONFIRMADA || e == PENDIENTE || e == CHECKIN;
}

struct Reserva {
    std::string id, dni, pasajero, codigoVuelo, origen, destino, fecha, hora;
    Clase clase;
    Categoria categoria;
    double precio;
    Estado estado;
    int idxVuelo;        // posicion del vuelo en el arreglo de vuelos
    long long llegada;   // orden de llegada a la lista de espera (desempate del heap)

    Reserva() : clase(C_ECONOMICA), categoria(CAT_REGULAR), precio(0),
                estado(PENDIENTE), idxVuelo(-1), llegada(0) {}
};

// Clave compuesta del arbol: fecha + hora + idReserva.
// Las fechas AAAA-MM-DD y horas HH:MM se ordenan correctamente como texto.
inline int compararClave(const Reserva* a, const Reserva* b) {
    int c = a->fecha.compare(b->fecha);
    if (c != 0) return c;
    c = a->hora.compare(b->hora);
    if (c != 0) return c;
    return a->id.compare(b->id);
}

inline void imprimirReserva(const Reserva* r) {
    std::cout << "  " << r->id << " | DNI " << r->dni << " | " << r->pasajero
              << " | " << r->codigoVuelo << " " << r->origen << "->" << r->destino
              << " | " << r->fecha << " " << r->hora
              << " | " << claseStr(r->clase) << "/" << categoriaStr(r->categoria)
              << " | " << r->precio << " | " << estadoStr(r->estado) << "\n";
}

#endif
