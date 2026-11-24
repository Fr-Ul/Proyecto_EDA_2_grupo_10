# Proyecto EDA II — Sistema de Reservas Aéreas con Lista de Espera

C++11, sin contenedores de la STL en el sistema (solo arreglos, `string`, punteros, `iostream`, `fstream`, `sstream`, `chrono`).

## Compilar y ejecutar

```bash
./compilar.sh            # o los 3 comandos g++ de abajo
./generador              # crea vuelos.csv, reservas_masivo.csv, vuelos_pequeno.csv, reservas_pequeno.csv
./reservas               # menú; opción 1 = carga masiva
./reservas vuelos_pequeno.csv reservas_pequeno.csv   # precarga y abre el menú
./benchmark              # tablas de tiempos y métricas para la sección Resultados
```

```bash
g++ -std=c++11 -O2 -o reservas   main.cpp Sistema.cpp TablaHash.cpp ArbolAVL.cpp MaxHeap.cpp
g++ -std=c++11 -O2 -o benchmark  benchmark.cpp Sistema.cpp TablaHash.cpp ArbolAVL.cpp MaxHeap.cpp
g++ -std=c++11 -O2 -o generador  generador.cpp
```

## Módulos

| Archivo | Contenido |
| --- | --- |
| `Reserva.h` | struct `Reserva`, enums, clave compuesta (`compararClave`) |
| `TablaHash.*` | Hash con encadenamiento; clave `idReserva`; tamaño primo 20011 |
| `ArbolAVL.*` | AVL con clave `fecha+hora+id`; inserción, eliminación, orden, rango con poda |
| `MaxHeap.*` | Max-heap en arreglo dinámico; `insertar`, `extraerMax`, `eliminar` en posición cualquiera |
| `Vuelo.h` | Vuelo + su lista de espera (heap) |
| `Sistema.*` | Las 10 funcionalidades y la consistencia entre las 3 estructuras |
| `main.cpp` | Menú |
| `benchmark.cpp` | Tiempos (repetidos con `<chrono>`), métricas y casos extremos |
| `generador.cpp` | Generador de datos (puede usar STL: no es parte del sistema) |

## Decisiones de diseño (para el informe y la sustentación)

- **Consistencia:** la tabla hash es dueña de los objetos `Reserva`; el árbol y los heaps guardan solo punteros. Un cambio de estado se ve en las tres estructuras sin duplicar datos. Para borrar: primero el árbol, luego la hash (que libera la reserva). `verificarConsistencia()` (opción 11) comprueba hash = árbol, `EnEspera` ⊂ heap y asientos ocupados por vuelo.
- **Orden de llegada:** campo `llegada` (contador global). En la carga masiva, el orden del archivo es el orden de llegada.
- **Desempate del heap:** categoría (Premium 2 > Frecuente 1 > Regular 0) y luego menor `llegada`.
- **Función hash:** polinómica `h = h·131 + c` módulo un primo (20011). Factor de carga ≈ 0.5 con 10,000 reservas.
- **AVL vs Red-Black (justificación sugerida):** el sistema es de lectura intensiva (búsquedas ordenadas y por rango). AVL es más estricto: altura ≤ 1.44·log₂(n) ≈ 19 para 10,000 datos (Red-Black hasta ≈ 27), a cambio de más rotaciones al insertar/borrar. Confirmen que esta es su decisión como grupo.
- **Rango de fechas:** O(log n + k). Se podan subárboles según la fecha del nodo; se reporta cuántos nodos se visitaron.
- **Cancelación:** también se permite cancelar una reserva en `CheckIn`; una `Cancelada` no se puede cancelar ni reactivar.
- **Registro (func. 2):** si hay asiento, el usuario elige `Confirmada` o `Pendiente` (por defecto `Pendiente`); si no, `EnEspera`.
- **Vuelo = fuente de verdad:** origen, destino, fecha y hora de la reserva se toman del vuelo (se reporta cuántas se corrigieron). Si el archivo trae más reservas con asiento que la capacidad, el exceso pasa a `EnEspera` (se reporta).
- **Archivo pequeño:** como los vuelos reales tienen 150–300 asientos, `reservas_pequeno.csv` va con `vuelos_pequeno.csv` (6 vuelos de capacidad 5 o 10, 3 llenos con 5–7 en espera, 2 con igual fecha y hora). Así la lista de espera se verifica a mano.
- **Repeticiones en el benchmark:** actualización, cancelación y eliminación se miden sobre 10 reservas distintas (una operación por reserva), porque no se pueden repetir sobre la misma.

## Pruebas pedidas en el enunciado → dónde se hacen

| Escenario | Cómo |
| --- | --- |
| Carga masiva | Menú 1 con `reservas_masivo.csv` (muestra hash, árbol y cada heap) |
| Búsqueda por ID | Menú 3 (`RES-000001`, una intermedia, `RES-010000`, un inexistente) |
| Orden cronológico | Menú 4 con N=20; verificar con `reservas_pequeno.csv` |
| Rango | Menú 5: una semana, un mes, y un rango sin resultados (p. ej. 2028-01-01 a 2028-01-31) |
| Inserción individual | Menú 2 (vuelo con cupo y vuelo lleno, ej. `LA100` en el masivo / `PQ001` en el pequeño) |
| Actualización / eliminación | Menús 6, 7 y 8; luego menú 11 |
| Lista de espera | Menú 9, 7 (promoción y cancelación de una `EnEspera` que no es la raíz), 9 de nuevo |
| Colisiones y orden desfavorable | `./benchmark` (sección "Casos extremos") |

Todo nodo creado con `new` se libera en los destructores (probado con AddressSanitizer, sin fugas).
