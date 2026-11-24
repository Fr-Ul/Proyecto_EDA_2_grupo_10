#ifndef SISTEMA_H
#define SISTEMA_H

#include <string>
#include "Reserva.h"
#include "Vuelo.h"
#include "TablaHash.h"
#include "ArbolAVL.h"

// Coordina hash + arbol + heaps y garantiza su consistencia:
//  - toda reserva esta en la hash y en el arbol
//  - toda reserva EnEspera esta ademas en el heap de su vuelo
class Sistema {
public:
    static const int MAX_VUELOS = 100;

    struct ResumenCarga {
        int leidas, enHash, enArbol, enHeaps, rechazadas, corregidas, reubicadas;
    };

    Vuelo vuelos[MAX_VUELOS];          // arreglo de tamano fijo (pocos vuelos)
    int numVuelos;
    TablaHash hash;
    ArbolAVL arbol;
    long long contadorLlegada;         // numera el orden de llegada a las listas de espera

    explicit Sistema(int tamHash = 20011);

    int buscarVuelo(const std::string& codigo) const;

    // 1. Carga masiva
    int cargarVuelos(const std::string& ruta);          // -1 si no abre el archivo
    ResumenCarga cargarReservas(const std::string& ruta);
    void mostrarHeaps() const;

    // 2. Registro (devuelve "" si todo OK, o el mensaje de error)
    std::string registrar(const Reserva& datos, Estado preferido, Reserva** creada = 0);

    // 3. Busqueda por ID (hash)
    Reserva* buscar(const std::string& id) const { return hash.buscar(id); }

    // 4. Listado cronologico (arbol)
    void listarCronologico(int k) const;

    // 5. Consulta por rango de fechas (arbol)
    void consultaRango(const std::string& fi, const std::string& ff);

    // 6. Actualizacion de estado (hash)
    std::string actualizarEstado(const std::string& id, Estado nuevo);

    // 7. Cancelacion (hash + heap). 'detalle' describe lo ocurrido.
    std::string cancelar(const std::string& id, std::string& detalle);

    // 8. Eliminacion fisica (hash + arbol)
    std::string eliminarFisica(const std::string& id);

    // 9. Lista de espera de un vuelo (heap)
    void listaEspera(const std::string& codigoVuelo) const;

    // 10. Estadisticas generales
    void estadisticas() const;

    // Extras para pruebas e informe
    void metricas() const;
    bool verificarConsistencia(bool imprimir = true) const;
};

#endif
