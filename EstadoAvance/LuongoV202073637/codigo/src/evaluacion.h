// evaluacion.h
// Funcion de evaluacion de un turno y de una solucion completa.
#pragma once
#include <vector>

#include "instancia.h"

// Peso de la penalizacion por minuto de violacion de una restriccion blanda.
// Es mayor que el costo de abrir un turno nuevo (P^min = 432), de modo que la
// busqueda siempre prefiere reparar una violacion antes que ahorrar costo.
constexpr double PESO_VIOLACION = 1000.0;
// Minutos equivalentes de violacion por cada discontinuidad espacial.
constexpr int MINUTOS_POR_SALTO = 60;

// Resultado de evaluar un turno (secuencia de tramos ordenada por inicio).
struct EvalTurno {
    double pagado = 0;       // tiempo pagado (con recargos)
    double cambiosLinea = 0; // numero de cambios de linea
    double violacion = 0;    // minutos de violacion de reglas laborales/espaciales
    double costo = 0;        // pagado + p_L*cambios + PESO_VIOLACION*violacion
    // Detalle (para depuracion y para la salida verbosa)
    int spread = 0, descanso = 0, nocturno = 0;
    double trabajado = 0, sobretiempo = 0;
    int saltos = 0;  // discontinuidades de lugar
};

// Evalua un turno. Supone (restriccion dura mantenida por la busqueda) que los
// tramos no se traslapan en el tiempo y estan ordenados por hora de inicio.
EvalTurno evaluarTurno(const Instancia& I, const std::vector<int>& turno);

// true si el tramo j puede insertarse en el turno sin traslape temporal.
// pos devuelve la posicion de insercion que mantiene el orden.
bool cabeSinTraslape(const Instancia& I, const std::vector<int>& turno, int j, int& pos);
