// solucion.h
// Representacion: asignacion directa tramo -> conductor (variables x_jk del modelo).
// Internamente se guarda como una lista de turnos; cada turno es la secuencia de
// tramos del conductor ordenada por hora de inicio (el orden queda determinado
// por los horarios, por lo que la asignacion define completamente la solucion).
#pragma once
#include <ostream>
#include <random>
#include <vector>

#include "evaluacion.h"
#include "instancia.h"

struct Solucion {
    std::vector<std::vector<int>> turnos;  // turnos[k] = tramos del conductor k
    std::vector<EvalTurno> eval;           // evaluacion cacheada de cada turno
    double costo = 0;                      // suma de eval[k].costo

    // Recalcula todas las evaluaciones desde cero.
    void evaluarCompleta(const Instancia& I);
    // Elimina conductores sin tramos.
    void compactar();

    double violacionTotal() const;
    bool esFactible() const { return violacionTotal() == 0; }
    // Valor de la funcion objetivo original (sin penalizaciones).
    double objetivo(const Instancia& I) const;
    // Vector tramo -> conductor (IDs desde 1), para la salida.
    std::vector<int> asignacion(const Instancia& I) const;
};

// Solucion inicial aleatoria: recorre los tramos en orden aleatorio y asigna cada
// uno a un conductor elegido al azar entre los que pueden recibirlo sin violar
// ninguna regla; si ninguno puede, abre un conductor nuevo.
Solucion solucionInicialAleatoria(const Instancia& I, std::mt19937& rng);

// Escribe la salida en el formato del curso:
//   <FO> <tiempo> <semilla> <iteraciones>
//
//   <ID tramo> <ID conductor>   (n lineas)
void escribirSalida(std::ostream& out, const Instancia& I, const Solucion& s, double tiempo,
                    unsigned semilla, long long iteraciones);

// Resumen legible de cada turno (opcion -v).
void escribirDetalle(std::ostream& out, const Instancia& I, const Solucion& s);
