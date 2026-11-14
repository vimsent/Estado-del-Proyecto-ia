// hillclimbing.h
// Hill-Climbing con Best Improvement y reinicios (HC-BI + Restart).
#pragma once
#include <random>

#include "instancia.h"
#include "solucion.h"

struct ConfigHC {
    double tiempoMax = 60.0;   // segundos totales (todas las reinicios)
    int maxReinicios = 1000000; // limite de reinicios
    bool usarMover = true;      // vecindario N1: mover un tramo a otro conductor
    bool usarCruce = true;      // vecindario N2: intercambio de sufijos entre turnos
    bool verbose = false;
};

struct ResultadoHC {
    Solucion mejor;
    long long iteraciones = 0;  // movimientos aplicados (suma sobre reinicios)
    int reinicios = 0;
    double tiempoMejor = 0;     // instante en que se encontro la mejor solucion
};

// Aplica HC-BI a s hasta llegar a un optimo local. Devuelve el numero de
// movimientos aplicados. Se detiene antes si se agota el tiempo (deadline).
long long hillClimbingBI(const Instancia& I, Solucion& s, const ConfigHC& cfg,
                         double deadline, double (*reloj)());

// Ciclo completo: solucion inicial aleatoria + HC-BI, repetido hasta agotar el
// tiempo o los reinicios; guarda la mejor solucion encontrada.
ResultadoHC hcbiConReinicios(const Instancia& I, std::mt19937& rng, const ConfigHC& cfg,
                             double (*reloj)());
